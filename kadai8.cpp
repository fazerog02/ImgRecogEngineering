#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <math.h>

using namespace std;

#define CHAR_NUM 46
#define COV_MATRIX_DIMENSION 196


struct Eigen {
    vector<double> values;
    vector<vector<double>> vectors;
};


void print2DimVec(vector<vector<double>> const& vec) {
    for(int i = 0; i < vec.size(); i++) {
        for(int j = 0; j < vec[0].size(); j++) {
            cout << vec[i][j] << " ";
        }
        cout << endl;
    }
}


vector<vector<vector<double>>> readCovFile(string const& file_name) {
    ifstream ifs(file_name, ios_base::in);
    if(!ifs) {
        cout << "file open error!" << endl;
        exit(EXIT_FAILURE);
    }

    string buf;
    int read_line_cnt = 0;

    vector<vector<vector<double>>> cov_of_each_chars(
        CHAR_NUM,
        vector<vector<double>>(
            COV_MATRIX_DIMENSION,
            vector<double>(
                COV_MATRIX_DIMENSION,
                0
            )
        )
    );

    int cov_i, cov_row_i, cov_col_i;
    while(getline(ifs, buf)) {
        cov_col_i = read_line_cnt % COV_MATRIX_DIMENSION;
        cov_row_i = read_line_cnt / COV_MATRIX_DIMENSION % COV_MATRIX_DIMENSION;
        cov_i = read_line_cnt / COV_MATRIX_DIMENSION / COV_MATRIX_DIMENSION % CHAR_NUM;
        cov_of_each_chars[cov_i][cov_row_i][cov_col_i] = stod(buf);
        read_line_cnt++;
    }

    return cov_of_each_chars;
}


pair<int, int> getMaxValuePosition(vector<vector<double>> const& cov) {
    pair<int, int> max_value_position = make_pair(0, 1);  // (y, x)
    for(int i = 0; i < cov.size(); i++) {
        for(int j = 0; j < cov[0].size(); j++) {
            if(i == j) continue;
            if(abs(cov[max_value_position.first][max_value_position.second]) < abs(cov[i][j])) {
                max_value_position.first = i;
                max_value_position.second = j;
            }
        }
    }

    return max_value_position;
}


bool isDiagonalMatrix(vector<vector<double>> const& matrix) {
    for(int i = 0; i < matrix.size(); i++) {
        for(int j = 0; j < matrix[0].size(); j++) {
            if(i == j) continue;
            // ### しきい値である1e-5以下だったら0とみなす ###
            if(abs(matrix[i][j]) > 1e-5) return false;
        }
    }
    return true;
}


Eigen getEigenByJacobi(vector<vector<double>>& cov) {
    pair<int, int> max_pos;
    int i, j;
    vector<vector<double>> p_sum, p_sum_copy, new_p_copy, cov_copy;
    vector<vector<double>> new_p(
        cov.size(),
        vector<double>(
            cov[0].size(),
            0
        )
    );
    double theta;
    int counter = 0;

    while(!isDiagonalMatrix(cov)) {
        max_pos = getMaxValuePosition(cov);
        i = max_pos.first;
        j = max_pos.second;

        if(cov[j][j] - cov[i][i] != 0){
            theta = (1.0/2) * atan(
                2.0 * cov[i][j] / (cov[j][j] - cov[i][i])
            );
        }else {
            theta = M_PI / 4.0;
        }

        for(int row = 0; row < new_p.size(); row++) {
            for(int col = 0; col < new_p[0].size(); col++) {
                if(row == col){
                    new_p[row][col] = 1;
                }else {
                    new_p[row][col] = 0;
                }
            }
        }
        new_p[i][i] = cos(theta);
        new_p[j][j] = cos(theta);
        new_p[i][j] = sin(theta);
        new_p[j][i] = -1.0 * sin(theta);

        if(counter == 0) {
            p_sum = new_p;
        }else {
            p_sum_copy = p_sum;
            for(int row = 0; row < p_sum.size(); row++) {
                p_sum[row][i] = 0;
                for(int c = 0; c < p_sum.size(); c++) {
                    p_sum[row][i] += p_sum_copy[row][c] * new_p[c][i];
                }
                p_sum[row][j] = 0;
                for(int c = 0; c < p_sum[j].size(); c++) {
                    p_sum[row][j] += p_sum_copy[row][c] * new_p[c][j];
                }
            }
        }

        // cout << endl << "##########" << endl;
        // cout << "theta: " << theta << endl;
        // cout << "(" << i << "," << j << ")" << endl;
        // cout << "----- rotate matrix -----" << endl;
        // print2DimVec(new_p);
        // cout << "-------------------------" << endl;
        // print2DimVec(cov);

        cov_copy = cov;
        for(int row = 0; row < cov.size(); row++) {
            cov[row][i] = 0;
            for(int c = 0; c < new_p.size(); c++) {
                cov[row][i] += cov_copy[row][c] * new_p[c][i];
            }
            cov[row][j] = 0;
            for(int c = 0; c < new_p.size(); c++) {
                cov[row][j] += cov_copy[row][c] * new_p[c][j];
            }
        }

        // 転置
        new_p_copy = new_p;
        for(int row = 0; row < new_p.size(); row++) {
            for(int col = 0; col < new_p[0].size(); col++) {
                if(row == col) continue;
                new_p[row][col] = new_p_copy[col][row];
            }
        }

        cov_copy = cov;
        for(int col = 0; col < cov[0].size(); col++) {
            cov[i][col] = 0;
            for(int c = 0; c < new_p.size(); c++) {
                cov[i][col] += new_p[i][c] * cov_copy[c][col];
            }
            cov[j][col] = 0;
            for(int c = 0; c < new_p.size(); c++) {
                cov[j][col] += new_p[j][c] * cov_copy[c][col];
            }
        }


        // cout << "----------" << endl;
        // print2DimVec(cov);
        // cout << cov[max_pos.second][max_pos.first] << endl;

        counter++;
    }

    cout << "###values###" << endl;
    vector<double> values;
    for(int row = 0; row < cov.size(); row++) {
        for(int col = 0; col < cov[0].size(); col++) {
            if(row == col) {
                values.push_back(cov[row][col]);
                cout << cov[row][col] << endl;
            }
        }
    }

    cout << endl << "###vectors###" << endl;
    counter = 0;
    vector<vector<double>> vectors;
    for(int col = 0; col < p_sum[0].size(); col++) {
        vectors.push_back(vector<double>());
        for(int row = 0; row < p_sum.size(); row++) {
            vectors[counter].push_back(p_sum[row][col]);
            cout << p_sum[row][col] << " ";
        }
        cout << endl;
        counter++;
    }

    Eigen eigen;
    eigen.values = values;
    eigen.vectors = vectors;

    return eigen;
}


int main() {
    string file_name;
    cout << "type file name: ";
    cin >> file_name;

    vector<vector<vector<double>>> cov_of_each_chars = readCovFile(file_name);

    // vector<vector<double>> a(
    //     4,
    //     vector<double>(
    //         4,
    //         0
    //     )
    // );
    // a[0][0] = 1;
    // a[0][1] = 2;
    // a[0][2] = 3;
    // a[0][3] = 4;

    // a[1][0] = 2;
    // a[1][1] = 5;
    // a[1][2] = 4;
    // a[1][3] = 0;

    // a[2][0] = 3;
    // a[2][1] = 4;
    // a[2][2] = 1;
    // a[2][3] = 1;

    // a[3][0] = 4;
    // a[3][1] = 0;
    // a[3][2] = 1;
    // a[3][3] = 2;

    // getEigenByJacobi(a);

    vector<Eigen> eigen_list;
    for(int i = 0; i < 1; i++) {
        eigen_list.push_back(getEigenByJacobi(cov_of_each_chars[i]));
    }
}
