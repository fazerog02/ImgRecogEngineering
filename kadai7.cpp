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
#define DEF_DIVIDE_BLOCK_SIZE 8


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
    cout << "}";
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


vector<vector<Def>> extractImgDef(vector<vector<char>>& img) {
    const int i_limit = img.size()/DEF_DIVIDE_BLOCK_SIZE-1;
    const int j_limit = img[0].size()/DEF_DIVIDE_BLOCK_SIZE-1;
    vector<vector<Def>> def(
        i_limit,
        vector<Def>(
            j_limit,
            {0, 0, 0, 0}
        )
    );

    for(int i = 0; i < i_limit; i++){
        for(int j = 0; j < j_limit; j++){
            int row_zero = i * img.size()/DEF_DIVIDE_BLOCK_SIZE;
            int col_zero = j * img[0].size()/DEF_DIVIDE_BLOCK_SIZE;
            for(int row = row_zero; row < row_zero + img.size()/DEF_DIVIDE_BLOCK_SIZE*2; row++){
                for(int col = col_zero; col < col_zero + img[0].size()/DEF_DIVIDE_BLOCK_SIZE*2; col++){
                    // 重み計算
                    int value = static_cast<int>(
                        min(
                            5 - abs(3.5 - static_cast<int>((row-row_zero)/2)),
                            5 - abs(3.5 - static_cast<int>((col-col_zero)/2))
                        )
                    );
                    // 右肩下がり
                    if(row - 1 >= 0 && col - 1 >= 0 && img[row-1][col-1] == '*') def[i][j].lower_right += value;
                    if(row + 1 < img.size() && col + 1 < img[0].size() && img[row+1][col+1] == '*') def[i][j].lower_right += value;
                    // 右肩上がり
                    if(row - 1 >= 0 && col + 1 < img[0].size() && img[row-1][col+1] == '*') def[i][j].upper_right += value;
                    if(row + 1 < img.size() && col - 1 >= 0 && img[row+1][col-1] == '*') def[i][j].upper_right += value;
                    // 垂直
                    if(row - 1 >= 0 && img[row-1][col] == '*') def[i][j].vertical += value;
                    if(row + 1 < img.size() && img[row+1][col] == '*') def[i][j].vertical += value;
                    // 水平
                    if(col + 1 < img[0].size() && img[row][col+1] == '*') def[i][j].horizontal += value;
                    if(col - 1 >= 0 && img[row][col-1] == '*') def[i][j].horizontal += value;
                }
            }
        }
    }

    return def;
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
    vector<vector<vector<Def>>> def_list;

    for(int i = 0; i < IMG_NUM; i++){
        images[i] = getImgFromImgRawArr(img_raw_arr, i);
        removeImgNoise(images[i]);
        normalizeImg(images[i]);
        extractImgOutline(images[i]);
        def_list.push_back(extractImgDef(images[i]));
    }

    for(int i = 0; i < IMG_NUM; i++){
        printImg(images[i]);
        for(int j = 0; j < def_list[i].size(); j++){
            for(int k = 0; k < def_list[i][0].size(); k++){
                printDef(def_list[i][j][k]);
                cout << " ";
            }
            cout << endl;
        }
    }


    return 0;
}


/*

{hor: 53, ver: 45, upr: 49, lowr: 51} {hor: 102, ver: 95, upr: 102, lowr: 99} {hor: 98, ver: 104, upr: 97, lowr: 99} {hor: 145, ver: 148, upr: 139, lowr: 135} {hor: 86, ver: 82, upr: 81, lowr: 83} {hor: 4, ver: 0, upr: 4, lowr: 4} {hor: 0, ver: 0, upr: 0, lowr: 0}
{hor: 146, ver: 153, upr: 154, lowr: 151} {hor: 107, ver: 99, upr: 105, lowr: 107} {hor: 105, ver: 103, upr: 108, lowr: 103} {hor: 59, ver: 74, upr: 67, lowr: 70} {hor: 30, ver: 42, upr: 41, lowr: 43} {hor: 6, ver: 6, upr: 7, lowr: 7} {hor: 0, ver: 0, upr: 0, lowr: 0}
{hor: 62, ver: 58, upr: 54, lowr: 54} {hor: 133, ver: 118, upr: 131, lowr: 131} {hor: 113, ver: 103, upr: 107, lowr: 106} {hor: 85, ver: 86, upr: 80, lowr: 83} {hor: 115, ver: 113, upr: 109, lowr: 116} {hor: 67, ver: 70, upr: 67, lowr: 69} {hor: 30, ver: 26, upr: 28, lowr: 24}
{hor: 25, ver: 21, upr: 23, lowr: 24} {hor: 102, ver: 96, upr: 101, lowr: 99} {hor: 86, ver: 87, upr: 86, lowr: 88} {hor: 89, ver: 89, upr: 88, lowr: 90} {hor: 113, ver: 116, upr: 113, lowr: 112} {hor: 158, ver: 159, upr: 153, lowr: 156} {hor: 120, ver: 121, upr: 120, lowr: 121}
{hor: 92, ver: 93, upr: 95, lowr: 95} {hor: 117, ver: 118, upr: 114, lowr: 122} {hor: 47, ver: 58, upr: 53, lowr: 55} {hor: 80, ver: 81, upr: 84, lowr: 84} {hor: 66, ver: 72, upr: 72, lowr: 67} {hor: 50, ver: 61, upr: 54, lowr: 54} {hor: 110, ver: 114, upr: 111, lowr: 114}
{hor: 134, ver: 131, upr: 135, lowr: 138} {hor: 145, ver: 143, upr: 149, lowr: 149} {hor: 108, ver: 109, upr: 103, lowr: 109} {hor: 104, ver: 108, upr: 109, lowr: 107} {hor: 33, ver: 39, upr: 37, lowr: 36} {hor: 50, ver: 49, upr: 48, lowr: 50} {hor: 141, ver: 148, upr: 141, lowr: 136}
{hor: 159, ver: 158, upr: 159, lowr: 163} {hor: 155, ver: 153, upr: 157, lowr: 158} {hor: 115, ver: 111, upr: 115, lowr: 110} {hor: 112, ver: 111, upr: 112, lowr: 111} {hor: 48, ver: 48, upr: 46, lowr: 49} {hor: 125, ver: 120, upr: 125, lowr: 123} {hor: 149, ver: 154, upr: 149, lowr: 142}

*/