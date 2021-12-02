#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <filesystem>

using namespace std;

#define IMG_FILE_SIZE_BYTE 512
#define IMG_NUM 20
#define MIN_BLOCK_SIZE 6


// 方向線素特徴量. (Directional Element Feature:DEF)
struct Def {
    int horizontal;
    int vertical;
    int upper_right;
    int lower_right;
};


void printImg(vector<vector<char>> const& img){
    /*
        画像データを表示する
    */

    for(int row = 0; row < IMG_FILE_SIZE_BYTE/8; row++){
        for(int col = 0; col < IMG_FILE_SIZE_BYTE/8; col++){
            cout << img[row][col];
        }
        cout << endl;
    }
    cout << endl;
}


void printDef(Def& def) {
    cout << "{";
    cout << "hor: " << def.horizontal << ", ";
    cout << "ver: " << def.vertical << ", ";
    cout << "upr: " << def.upper_right << ", ";
    cout << "lowr: " << def.lower_right;
    cout << "}" << endl;
}


vector<unsigned char> readImgFile(string const& file_path){
    /*
        バイナリファイルからunsigned charでデータを読み取る
    */

    size_t size = filesystem::file_size(file_path);
    ifstream ifs(file_path, ios::in | ios::binary);
    if(!ifs){
        // ファイルを開けなかったら終了
        cout << "file open error!" << endl;
        exit(EXIT_FAILURE);
    }

    vector<unsigned char> data(size);
    ifs.read(reinterpret_cast<char *>(&data[0]), size);

    return data;
}


vector<vector<char>> getImgFromImgRawArr(vector<unsigned char> const& img_raw_arr, int index){
    /*
        画像を表すunsigned charの配列から，*とoで表す視覚的に理解できる画像データを生成する
    */

    // 画像サイズは64*64
    vector<vector<char>> img_char_arr(
        IMG_FILE_SIZE_BYTE/8,
        vector<char>(
            IMG_FILE_SIZE_BYTE/8,
            '.'
        )
    );

    const int row_size = img_char_arr.size();
    const int col_size = img_char_arr[0].size();

    unsigned char mask;
    for(int row = index*row_size; row < (index+1)*row_size; row++){
        for(int col = 0; col < col_size/8; col++){  // charは1byte(=8bit)なのでループは8回でいい(8*char8個 = 64bit)
            // ucharのバイナリを見て0,1を*,oとして取り出す
            mask = 0x80;  // 2進数で10000000
            for(int i = 0; i < 8; i++){
                if(img_raw_arr[row*8 + col] & mask){
                    img_char_arr[row - index*row_size][col*8 + i] = '*';
                }else{
                    img_char_arr[row - index*col_size][col*8 + i] = 'o';
                }
                // 1ビット右にずらす(unsignedなので右は0埋め)
                // これを8回ずらせば&ですべてのビットに対して1かどうか確認できる
                // ex) 10000000 -> 01000000
                mask = mask >> 1;
            }
        }
    }

    return img_char_arr;
}


void extractImgOutline(vector<vector<char>>& img){
    /*
        画像から輪郭を抽出して"""上書きする"""
    */

    vector<pair<int, int>> outline_positions;
    outline_positions.clear();

    static const int directions[4][2] = {
        {0, -1},  // 上
        {1, 0},  // 右
        {0, 1},  // 下
        {-1, 0}  // 左
    };
    int target_row, target_col;
    bool is_outline;

    // 輪郭の座標を抽出する
    const int row_size = img.size();
    const int col_size = img[0].size();
    for(int row = 0; row < row_size; row++){
        for(int col = 0; col < col_size; col++){
            if(img[row][col] != '*') continue;

            is_outline = false;
            for(int i = 0; i < 4; i++){
                target_row = row + directions[i][1];
                target_col = col + directions[i][0];
                if(
                    target_row < 0 ||
                    target_row >= row_size ||
                    target_col < 0 ||
                    target_col >= col_size
                ){
                    break;
                }

                if(img[target_row][target_col] != '*'){
                    is_outline = true;
                    break;
                }
            }

            if(is_outline){
                outline_positions.push_back(make_pair(row, col));
            }
        }
    }

    // とりあえず全て白画素で初期化
    img = vector<vector<char>>(
        img.size(),
        vector<char>(
            img[0].size(),
            'o'
        )
    );

    for(int i = 0; i < outline_positions.size(); i++){
        img[outline_positions[i].first][outline_positions[i].second] = '*';
    }
}


void normalizeImg(vector<vector<char>>& img){
    /*
        画像を端までいっぱいに拡大するして"""上書きする""""
    */

    // オフセット: top(x, y), bottom(x, y)
    vector<pair<int, int>> offsets(
        2,
        make_pair(-1, -1)
    );
    pair<int, int> size;
    bool is_blank_line;
    int target_row, target_col;

    const int row_size = img.size();
    const int col_size = img[0].size();

    // 横方向のオフセットを探索
    for(int i = 0; i < 2; i++){
        for(int col = 0; col < col_size; col++){
            is_blank_line = true;
            for(int row = 0; row < row_size; row++){
                // これをすると上からと下からの両方に対応できる
                target_row = (img.size()-1) * i + (i == 0 ? row : -1*row);
                target_col = (img[0].size()-1) * i + (i == 0 ? col : -1*col);
                if(img[target_row][target_col] != 'o'){
                    is_blank_line = false;
                    break;
                }
            }
            if(!is_blank_line){
                offsets[i].first = col;
                break;
            }
        }
    }

    // 縦方向のオフセットを探索
    for(int i = 0; i < 2; i++){
        for(int row = 0; row < row_size; row++){
            is_blank_line = true;
            for(int col = 0; col < col_size; col++){
                // これをすると上からと下からの両方に対応できる
                target_row = (row_size-1) * i + (i == 0 ? row : -1*row);
                target_col = (col_size-1) * i + (i == 0 ? col : -1*col);
                if(img[target_row][target_col] != 'o'){
                    is_blank_line = false;
                    break;
                }
            }
            if(!is_blank_line){
                offsets[i].second = row;
                break;
            }
        }
    }

    size.first = row_size - offsets[0].first - offsets[1].first;
    size.second = col_size - offsets[0].second - offsets[1].second;

    vector<vector<char>> new_img(
        row_size,
        vector<char>(
            col_size,
            '.'
        )
    );
    int refer_row, refer_col;
    for(int row = 0; row < row_size; row++){
        for(int col = 0; col < col_size; col++){
            refer_row = static_cast<int>(
                static_cast<double>(row) * static_cast<double>(size.second) / static_cast<double>(row_size) + static_cast<double>(offsets[0].second) + 0.5
            );
            refer_col = static_cast<int>(
                static_cast<double>(col) * static_cast<double>(size.first) / static_cast<double>(col_size) + static_cast<double>(offsets[0].first) + 0.5
            );
            new_img[row][col] = img[refer_row][refer_col];
        }
    }

    img = new_img;
}


void getBlockPositions(vector<vector<char>>& img, vector<vector<bool>>& flags, vector<pair<int, int>>& positions, pair<int, int> position) {
    /*
        再帰的に黒画素の塊を探索して構成画素の座標を取得する
        ※positionとpositionのpairは(y, x)を表す
    */

    static const int directions[4][2] = {
        // """{y, x}"""
        {-1, 0},  // 上
        {0, 1},  // 右
        {1, 0},  // 下
        {0, -1}  // 左
    };

    flags[position.first][position.second] = true;
    positions.push_back(
        make_pair(position.first, position.second)
    );

    int target_row, target_col;
    for(int i = 0; i < 4; i++) {
        target_row = position.first + directions[i][0];
        target_col = position.second + directions[i][1];
        if(
            target_row < 0 ||
            target_row >= img.size() ||
            target_col < 0 ||
            target_col >= img[0].size()
        ){
            continue;
        }
        if(img[target_row][target_col] != '*' || flags[target_row][target_col]) continue;

        getBlockPositions(img, flags, positions, make_pair(target_row, target_col));
    }
}


void removeImgNoise(vector<vector<char>>& img) {
    /*
        画像のノイズを除去して"""上書きする""" (塊が5個以下の黒画素で構成されていたら削除)
    */

   const int row_size = img.size();
   const int col_size = img[0].size();
    vector<vector<bool>> flags = vector<vector<bool>>(
       row_size,
       vector<bool>(
           col_size,
           false
       )
    );
    vector<pair<int, int>> positions;

    for(int row = 0; row < row_size; row++){
        for(int col = 0; col < col_size; col++){
            // 黒画素が見つかったらそこから再帰で掘っていって塊を見つける
            if(img[row][col] == '*' && !flags[row][col]) {
                positions.clear();
                getBlockPositions(img, flags, positions, make_pair(row, col));

                // 塊の黒画素が5個以下だったら削除
                if(positions.size() < MIN_BLOCK_SIZE){
                    for(int i = 0; i < positions.size(); i++){
                        img[positions[i].first][positions[i].second] = 'o';
                    }
                }
            }
        }
    }
}


Def extractImgDef(vector<vector<char>>& img) {

}


int main(){
    int n;
    cout << "type file number: ";
    cin >> n;
    cin.get();  // enter吸収

    // file pathの加工
    ios::fmtflags current_flag = std::cout.flags();  // 現在のcoutの設定を保管
    ostringstream oss;
    oss << "./samples/" << setw(2) << setfill('0') << n << ".img";  // 2桁の0埋め
    string file_path = oss.str();
    cout.flags(current_flag);  // coutの設定を戻す

    // ファイルを読み込み
    vector<unsigned char> img_raw_arr = readImgFile(file_path);
    if(img_raw_arr.size() != IMG_FILE_SIZE_BYTE * IMG_NUM){
        // 画像20枚分のデータを読み込めていなかったら終了
        cout << "file size error!" << endl;
        exit(EXIT_FAILURE);
    }

    vector<vector<vector<char>>> images(
        IMG_NUM,
        vector<vector<char>>(
            IMG_FILE_SIZE_BYTE/8,
            vector<char>(
                IMG_FILE_SIZE_BYTE/8,
                '.'
            )
        )
    );
    vector<Def> def_list;
    for(int i = 0; i < IMG_NUM; i++){
        images[i] = getImgFromImgRawArr(img_raw_arr, i);
        removeImgNoise(images[i]);
        normalizeImg(images[i]);
        extractImgOutline(images[i]);
    }

    for(int i = 0; i < IMG_NUM; i++) printImg(images[i]);

    return 0;
}
