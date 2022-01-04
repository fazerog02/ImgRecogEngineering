#include <iostream>
#include <vector>
#include <string>
#include <math.h>
#include <fstream>
#include <iomanip>
#include <sstream>

using namespace std;

#define CHAR_NUM 46
#define FILE_DATA_NUM 20
#define COV_MATRIX_DIMENSION 196

struct Eigen {
    vector<double> values;
    vector<vector<double>> vectors;
};


vector<vector<double>> readAverageFile() {
    ifstream ifs("average.dic", ios::in);
    if(!ifs) {
        cout << "read average file error!" << endl;
        exit(EXIT_FAILURE);
    }

    vector<vector<double>> avarages(
        CHAR_NUM,
        vector<double>(
            COV_MATRIX_DIMENSION,
            0
        )
    );

    int counter = 0;
    string buf;
    double buf_double;
    while(getline(ifs, buf)) {
        buf_double = stod(buf);
        avarages[counter/COV_MATRIX_DIMENSION][counter%COV_MATRIX_DIMENSION] = buf_double;
        counter++;
    }

    return avarages;
}


vector<Eigen> readEigens() {
    vector<Eigen> eigens(
        CHAR_NUM,
        Eigen{
            vector<double>(COV_MATRIX_DIMENSION, 0),
            vector<vector<double>>(
                COV_MATRIX_DIMENSION,
                vector<double>(
                    COV_MATRIX_DIMENSION,
                    0
                )
            )
        }
    );

    string file_path;
    ifstream ifs;
    stringstream ss;
    string buf;
    string ss_buf;
    double buf_double;
    int counter;

    for(int i = 0; i < CHAR_NUM; i++) {
        file_path = "./eigens/" + to_string(i) + ".txt";

        ifs.open(file_path, ios::in);
        if(!ifs) {
            cout << "read eigen file error!" << endl;
            exit(EXIT_FAILURE);
        }

        counter = 0;
        while(getline(ifs, buf)) {
            ss.str(buf);
            ss.clear();
            while(getline(ss, ss_buf, ',')) {
                buf_double = stod(ss_buf);
                if(counter < COV_MATRIX_DIMENSION) {
                    eigens[i].values[counter] = buf_double;
                }else {
                    eigens[i].vectors[counter/COV_MATRIX_DIMENSION - 1][counter%COV_MATRIX_DIMENSION] = buf_double;
                }
                counter++;
            }
        }
        ifs.close();
    }

    return eigens;
}


vector<vector<vector<double>>> readTests() {
    vector<vector<vector<double>>> tests(
        CHAR_NUM,
        vector<vector<double>>(
            FILE_DATA_NUM,
            vector<double>(
                COV_MATRIX_DIMENSION,
                0
            )
        )
    );

    ios::fmtflags current_flag = std::cout.flags();  // 現在のcoutの設定を保管
    ostringstream oss;
    string file_path;
    ifstream ifs;
    string buf;
    double buf_double;
    int counter;

    for(int i = 1; i <= CHAR_NUM; i++) {
        // file pathの加工
        oss.str("");
        oss << "./tests/unknown" << setw(2) << setfill('0') << i << ".txt";  // 2桁の0埋め
        file_path = oss.str();

        ifs.open(file_path, ios::in);
        if(!ifs) {
            cout << "read test file error!" << endl;
            exit(EXIT_FAILURE);
        }

        counter = 0;
        while(getline(ifs, buf)) {
            buf_double = stod(buf);
            tests[i-1][counter/COV_MATRIX_DIMENSION][counter%COV_MATRIX_DIMENSION] = buf_double;
            counter++;
        }
        ifs.close();
    }
    cout.flags(current_flag);  // coutの設定を戻す

    return tests;
}


void subVector(vector<double> const& a, vector<double> const& b, vector<double>& ans) {
    if(a.size() != b.size()) return;

    for(int i = 0; i < a.size(); i++) {
        ans[i] = a[i] - b[i];
    }
}


double multVec(vector<double> const& a, vector<double> const& b) {
    if(a.size() != b.size()) return 0;

    double ans = 0;
    for(int i = 0; i < a.size(); i++) {
        ans += a[i] * b[i];
    }

    return ans;
}


void calcMahalanobis(vector<double> const& feature, vector<vector<double>> const& avarages, vector<Eigen> const& eigens, double bias, vector<double>& results) {
    int char_num = eigens.size();
    vector<double> sub_tmp(
        feature.size(),
        0
    );

    int values_num;
    for(int i = 0; i < char_num; i++) {
        values_num = eigens[i].values.size();
        for(int j = 0; j < values_num; j++) {
            subVector(feature, avarages[i], sub_tmp);
            results[i] += pow(
                multVec(
                    sub_tmp,
                    eigens[i].vectors[j]
                ),
                2.0
            ) / (eigens[i].values[j] + bias);
        }
    }
}


int getMaxIndex(vector<double> const& vec) {
    int ans = 0;
    for(int i = 1; i < vec.size(); i++) {
        if(vec[ans] < vec[i]) ans = i;
    }

    return ans;
}


int main() {
    cout << "loading avarage..." << endl;
    vector<vector<double>> avarages = readAverageFile();

    cout << "loading eigens..." << endl;
    vector<Eigen> eigens = readEigens();

    cout << "loading tests..." << endl;
    vector<vector<vector<double>>> tests = readTests();

    cout << "file loading completed" << endl << endl;

    int bias = 1000;
    vector<double> results(
        CHAR_NUM,
        0
    );
    int result;
    int total_correct = 0;
    int total = 0;
    int local, local_correct;
    string hiragana[46] = {
        "あ","い","う","え","お",
        "か","き","く","け","こ",
        "さ","し","す","せ","そ",
        "た","ち","つ","て","と",
        "な","に","ぬ","ね","の",
        "は","ひ","ふ","へ","ほ",
        "ま","み","む","め","も",
        "や","ゆ","よ",
        "ら","り","る","れ","ろ",
        "わ","を","ん"
    };

    cout << "----- start recognizing -----" << endl;
    for(int i = 0; i < tests.size(); i++) {
        local = 0;
        local_correct = 0;
        for(int j = 0; j < tests[i].size(); j++) {
            calcMahalanobis(tests[i][j], avarages, eigens, bias, results);
            result = getMaxIndex(results);

            if(result == i) {
                local_correct++;
                total_correct++;
            }
            local++;
            total++;
        }
        cout << "「" << hiragana[i] << "」: ";
        cout << local_correct << "/" << local << "(" << (1.0*local_correct)/(1.0*local)*100 << "%)" << endl;
    }
    cout << "total result: ";
    cout << total_correct << "/" << total << "(" << (1.0*total_correct)/(1.0*total)*100 << "%)" << endl;
}
