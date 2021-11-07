#include <iostream>
#include <vector>

using namespace std;

int main(){
    vector<vector<int>> img(8, vector<int>(8, 0));
    vector<vector<int>> after_img(6, vector<int>(6, 0));

    for(int i = 2; i < 6; i++) img[i][2] = 18;
    for(int i = 2; i < 4; i++) img[i][3] = 18;
    for(int i = 3; i < 6; i++) img[4][i] = 9;
    for(int i = 3; i < 6; i++) img[5][i] = 9;

    double sum;
    int direc[9][2] = {
        {-1, -1},
        {-1, 0},
        {-1, 1},
        {0, 1},
        {1, 1},
        {1, 0},
        {1, -1},
        {0, -1},
        {0, 0}
    };
    for(int i = 1; i < 7; i++){
        for(int j = 1; j < 7; j++){
            sum = 0;
            for(int k = 0; k < 9; k++){
                sum += img[i + direc[k][1]][j + direc[k][0]];
            }
            after_img[i-1][j-1] = (int)(sum / 9.0 + 0.5);
        }
    }

    for(int i = 0; i < 6; i++){
        for(int j = 0; j < 6; j++){
            cout << "[" << after_img[i][j] << "]";
        }
        cout << endl;
    }

    return 0;
}