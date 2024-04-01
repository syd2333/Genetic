#include <vector>
#include <iostream>
#include <random>
#include <ctime>
#include <cstdlib>

using namespace std;

class Queen {
private:
    vector<int> positions;
    double fitness;
    int boardSize;
    int id;
    static int nextID; // 静态私有成员，用于生成唯一ID

public:
    Queen(int n) : fitness(0.0), id(nextID++), boardSize(n) {
        positions.resize(n);
        for (int i = 0; i < n; i++) {
            positions[i] = i;
        }
        random_shuffle(positions.begin(), positions.end());
    }

    Queen(const vector<int>& positions) : boardSize(positions.size()), fitness(0.0), id(nextID++) {
        this->positions = positions;
    }

    double getFitness() {
        return fitness;
    }

    vector<int> getPositions() {
        return positions;
    }

    int getBoardSize() const {
        return boardSize;
    }

    void setPositions(const vector<int>& newPositions) {
        positions = newPositions;
    }

    int getID() {
        return id;
    }

    void calculateFitness() {
        int maxConflicts = positions.size() * (positions.size() - 1) / 2;
        int conflicts = 0;

        for (int i = 0; i < positions.size() - 1; i++) {
            for (int j = i + 1; j < positions.size(); j++) {
                if (abs(positions[i] - positions[j]) == abs(i - j)) {
                    conflicts++;
                }
            }
        }

        fitness = maxConflicts - conflicts;
    }

    void mutate() {
        int n = positions.size();
        int times = positions.size() / 3 + 1;

        for (int i = 0; i < times; i++) {
            int idx1 = rand() % n;
            int idx2;
            do {
                idx2 = rand() % n;
            } while (idx1 == idx2);

            swap(positions[idx1], positions[idx2]);
        }
    }

    void printPositions() {
        cout << "皇后位置如下：" << endl;
        for (int i = 0; i < positions.size(); i++) {
            cout << "第" << i + 1 << "个皇后：行：" << positions[i] + 1 << "，列：" << i + 1 << endl;
        }
    }
};

int Queen::nextID = 0;

class Population {
private:
    vector<Queen> queens;
    vector<double> Probabilities;
    int populationSize;

public:
    Population(int size, int n) : populationSize(size), Probabilities(size, 0.0) {
        for (int i = 0; i < populationSize; i++) {
            queens.emplace_back(Queen(n));
        }
    }

    Queen getBestQueen() {
        Queen bestQueen = queens[0];
        for (Queen& queen : queens) {
            if (queen.getFitness() > bestQueen.getFitness()) {
                bestQueen = queen;
            }
        }

        return bestQueen;
    }

    void calculateFitness() {
        for (Queen& queen : queens) {
            queen.calculateFitness();
        }
    }

    void calculateProbabilities() {
        double sumOfFitness = 0;
        for (Queen& queen : queens) {
            sumOfFitness += queen.getFitness();
        }

        for (int i = 0; i < populationSize; i++) {
            Probabilities[i] = queens[i].getFitness() / sumOfFitness;
        }
    }

    Queen rouletteWheelSelection() {
        double randValue = (double)rand() / RAND_MAX;
        double sum = 0;
        for (int i = 0; i < populationSize; i++) {  //用顺式结构实现了轮盘的效果
            sum += Probabilities[i];
            if (randValue <= sum) {
                return queens[i];
            }
        }

        return queens.back();
    }

    void fixPositions(vector<int>& positions) {
        int n = positions.size();
        vector<bool> isOccupied(n, false);
        vector<int> duplicateIndices, availableColumns;

        for (int i = 0; i < n; ++i) {
            if (isOccupied[positions[i]]) {
                duplicateIndices.push_back(i); // 记录重复位置的索引
            }
            else {
                isOccupied[positions[i]] = true;
            }
        }

        for (int i = 0; i < n; ++i) {
            if (!isOccupied[i]) {
                availableColumns.push_back(i);
            }
        }

        random_shuffle(availableColumns.begin(), availableColumns.end());

        for (int index : duplicateIndices) {
            if (!availableColumns.empty()) {
                positions[index] = availableColumns.back(); // 分配一个随机未占据列
                availableColumns.pop_back(); // 从列表中移除这个列
            }
        }
    }

    void copulation() {
        calculateFitness();

        calculateProbabilities();

        vector<Queen> newQueens;
        double crossoverRate = 0.5;

        for (int i = 0; i < populationSize / 2; i++) {
            Queen parent1 = rouletteWheelSelection();
            Queen parent2 = rouletteWheelSelection();

            while (parent1.getID() == parent2.getID()) {
                parent2 = rouletteWheelSelection();
            }

            // 获取父代的棋盘位置
            vector<int> positions1 = parent1.getPositions();
            vector<int> positions2 = parent2.getPositions();

            double roll = static_cast<double>(rand()) / RAND_MAX;
            if (roll < crossoverRate) {
                int crossoverPoint = rand() % parent1.getBoardSize();
                for (int j = crossoverPoint; j < parent1.getBoardSize(); j++) {
                    swap(positions1[j], positions2[j]);
                }

                // 交叉操作后纠正数组
                fixPositions(positions1);
                fixPositions(positions2);
            }

            Queen queen1 = Queen(positions1);
            Queen queen2 = Queen(positions2);

            queen1.mutate();
            queen2.mutate();

            //变异
            newQueens.emplace_back(queen1);
            newQueens.emplace_back(queen2);
        }
        
        queens = newQueens;
    }

    int getPopulationSize() {
        return populationSize;
    }
};

int main() {
    srand(std::time(0));

    int n = 10; 
    int populationSize = 100;
    int generations = 1000;
    

    cout << "输入皇后数量:";
    cin >> n;
    cout << "依次输入种群中个体数量和迭代次数(例如:'10000 1000'意思是10000只个体迭代1000次):";
    cin >> populationSize >> generations;

    int optimalFitness = n * (n - 1) / 2;

    Population population(populationSize, n);

    Queen bestSoFar(n); 
    bestSoFar.calculateFitness();

    for (int i = 0; i < generations; i++) {
        population.copulation();

        population.calculateFitness();
        Queen currentBest = population.getBestQueen();

        if (currentBest.getFitness() > bestSoFar.getFitness()) {
            bestSoFar = currentBest; // 更新最优解
        }

        cout << "第" << i + 1 << "代的最优解适应度：" << currentBest.getFitness() << endl;

        if (currentBest.getFitness() == optimalFitness) {
            cout << "在第" << i + 1 << "代找到正确解。" << endl;
            break;
        }
    }

    if (bestSoFar.getFitness() != optimalFitness) {
        cout << "在" << generations << "代内未找到最佳解。" << endl;
        cout << "迄今为止找到的最优解的适应度为：" << bestSoFar.getFitness() << endl;
    }

    cout << "迄今为止找到的尽可能最优解的位置：" << endl;
    bestSoFar.printPositions();
    cout << endl;

    system("pause");
    return 0;
}
