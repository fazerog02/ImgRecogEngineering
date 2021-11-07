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


vector<unsigned char> readImgFile(string const& file_path){
    size_t size = filesystem::file_size(file_path);
    ifstream ifs(file_path, ios::in | ios::binary);
    if(!ifs){
        cout << "file open error!" << endl;
        exit(EXIT_FAILURE);
    }

    vector<unsigned char> data(size);
    ifs.read(reinterpret_cast<char *>(&data[0]), size);
    
    return data;
}


void printCharAsBinary(unsigned char img_byte){
    unsigned char mask = 0x80;  // 2進数で10000000
    for(int _i = 0; _i < 8; _i++){
        if(img_byte & mask){
            cout << "*";
        }else{
            cout << "o";
        }
        // 1ビット右にずらす(unsignedなので右は0埋め)
        // これを8回ずらせば&ですべてのビットに対して1かどうか確認できる
        // ex) 10000000 -> 01000000
        mask = mask >> 1;
    }
}


void printImgArr(vector<unsigned char>& img_raw_arr, int index){
    // 画像サイズは64*64
    for(int i = index*IMG_FILE_SIZE_BYTE/8; i < (index+1)*IMG_FILE_SIZE_BYTE/8; i++){
        for(int j = 0; j < IMG_FILE_SIZE_BYTE/8/8; j++){  // charは1byte(=8bit)なのでループは8回でいい(8*char8個 = 64bit)
            printCharAsBinary(img_raw_arr[i*8 + j]);
        }
        cout << endl;
    }
}


int main(){
    int n;
    cout << "type file number: ";
    cin >> n;
    cin.get();  // enter吸収

    // file pathの加工
    ios::fmtflags current_flag = std::cout.flags();
    ostringstream oss;
    oss << "./samples/" << setw(2) << setfill('0') << n << ".img";
    string file_path = oss.str();
    cout.flags(current_flag);

    // ファイルを読み込み
    vector<unsigned char> img_raw_arr = readImgFile(file_path);
    if(img_raw_arr.size() != IMG_FILE_SIZE_BYTE * IMG_NUM){
        cout << "file size error!" << endl;
        exit(EXIT_FAILURE);
    }
    
    // 文字画像を20個表示
    for(int i = 0; i < IMG_NUM; i++){
        printImgArr(img_raw_arr, i);
        if(i < IMG_NUM - 1){
            cout << endl;
            cin.get();  // enterを押されるまで待つ
        }
    }

    return 0;
}
