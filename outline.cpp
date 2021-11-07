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

    unsigned char mask;
    for(int row = index*IMG_FILE_SIZE_BYTE/8; row < (index+1)*IMG_FILE_SIZE_BYTE/8; row++){
        for(int col = 0; col < IMG_FILE_SIZE_BYTE/8/8; col++){  // charは1byte(=8bit)なのでループは8回でいい(8*char8個 = 64bit)
            // ucharのバイナリを見て0,1を*,oとして取り出す
            mask = 0x80;  // 2進数で10000000
            for(int i = 0; i < 8; i++){
                if(img_raw_arr[row*8 + col] & mask){
                    img_char_arr[row - index*IMG_FILE_SIZE_BYTE/8][col*8 + i] = '*';
                }else{
                    img_char_arr[row - index*IMG_FILE_SIZE_BYTE/8][col*8 + i] = 'o';
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


void outline(vector<vector<char>>& img){
    /*
        画像から輪郭を抽出して"""上書きする"""
    */

    vector<pair<int, int>> outline_positions;
    outline_positions.clear();

    int directions[4][2] = {
        {0, -1},  // 上
        {1, 0},  // 右
        {0, 1},  // 下
        {-1, 0}  // 左
    };
    int target_row, target_col;
    bool is_outline;

    // 輪郭の座標を抽出する
    for(int row = 0; row < IMG_FILE_SIZE_BYTE/8; row++){
        for(int col = 0; col < IMG_FILE_SIZE_BYTE/8; col++){
            if(img[row][col] != '*') continue;

            is_outline = false;
            for(int i = 0; i < 4; i++){
                target_row = row + directions[i][1];
                target_col = col + directions[i][0];
                if(
                    target_row < 0 ||
                    target_row >= IMG_FILE_SIZE_BYTE/8 ||
                    target_col < 0 ||
                    target_col >= IMG_FILE_SIZE_BYTE/8
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
        IMG_FILE_SIZE_BYTE/8,
        vector<char>(
            IMG_FILE_SIZE_BYTE/8,
            'o'
        )
    );

    for(int i = 0; i < outline_positions.size(); i++){
        img[outline_positions[i].first][outline_positions[i].second] = '*';
    }
}


int main(){
    int n;
    cout << "type file number: ";
    cin >> n;
    cin.get();  // enter吸収

    // file pathの加工
    ios::fmtflags current_flag = std::cout.flags();  // 現在のcoutの設定を保管
    ostringstream oss;
    oss << "./images/" << setw(2) << setfill('0') << n << ".img";  // 2桁の0埋め
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
    for(int i = 0; i < IMG_NUM; i++){
        images[i] = getImgFromImgRawArr(img_raw_arr, i);
        outline(images[i]);
    }

    for(int i = 0; i < IMG_NUM; i++) printImg(images[i]);

    return 0;
}