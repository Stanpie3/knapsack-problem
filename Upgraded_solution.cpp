#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include "profile.h"

using namespace std;

struct Node {
    int number = 1;
    float Bound = 0;
    int bound_item_index = 0;
    set<int> I, E;
    vector<float> solution;
};

bool operator == (const Node& lhs, const Node& rhs) {
    return lhs.number == rhs.number;
}

void remove_a_from_b(const set<int>& A, vector<int>& b) {
    for (auto a: A)
    {
        auto iter = std::find(b.begin(), b.end(), a);
        if (iter != b.end())
        {
            b.erase(iter);
        }
    }
}

void bound_for_node(Node& node, const vector<vector<int>>& Subscripts,
    const vector<int>& v, const vector<vector<int>>& A, const vector<int>& b) {
    vector<float> Z;            //Задаём вектора Z и bound_items - в них будут отходить границы и граничные элементы каждой операции
    vector<int> bound_items;
    vector<vector<float>> Solutions;
    Solutions.assign(b.size(), vector<float>(v.size()));      // Создаём вектор solutions. В него будем записывать решения для каждого ограничения, минимальное записываем в соотв. ноду

    for (int j = 0; j < b.size(); j++) {
        float sum = 0;
        float z = 0;
        for (auto c : node.I) {             //Заранее подсчитываем влияние на сумму и границу фиксированных в 1 элементов
            sum += A[j][c];
            z += v[c];
            Solutions[j][c] = 1.0;
        }
        if (sum > b[j]) {                   //Если эта сумма сама по себе нарушает ограничение, отбрасываем ветвь
            node.Bound = -sum;
            node.bound_item_index = 0;
            fill(node.solution.begin(), node.solution.end(), 0.0);
            return;
        }
        vector<int> Idx(Subscripts[j]); // Создаём вектор индексов для итерации (изначально - строка Subscr[j]) и удаляем из него все фиксированные элементы. 
        remove_a_from_b(node.E, Idx);
        remove_a_from_b(node.I, Idx);
        auto i = Idx.begin();
        float coef = 0;
        while (sum < b[j]) {
            if (i != Idx.end()) {
                if (sum + A[j][*i] < b[j]) {          //Если сумма с элементом меньше ограничения, прибавляем
                    sum += A[j][*i];
                    z += v[*i];
                    Solutions[j][*i] = 1.0;
                    i++;
                }
                else if (sum + A[j][*i] == b[j]) { //В случае равенства мы получили достижимую границу, отмечаем это
                    z += v[*i];
                    Solutions[j][*i] = 1.0;
                    bound_items.push_back(-1);  // Показатель достижимости (проходим по ограничению, так как в точности достигли раницы)
                    break;
                }
                else {
                    coef = (b[j] - sum) / A[j][*i];       //Находим коэффициент для границы из ограничения
                    Solutions[j][*i] = coef;
                    bound_items.push_back(*i);            //Добавляем граничный элемент и его вес с коэффицентом к оцениваемой функции
                    z += v[*i] * coef;
                    break;
                }
            }
            else {
                bound_items.push_back(-1);
                break;
            }
        }
        Z.push_back(z);         //Пушим в вектор z каждую промежуточную границу 
    }
    /*for (auto z : Z) {
        cout << z << " ";
    }
    cout << endl;*/
    auto it = min_element(Z.begin(), Z.end());          //Находим минимум среди границ, берем соответствующий ей граничный элемент
    node.Bound = *it;
    node.bound_item_index = bound_items.at(distance(Z.begin(), it));
    node.solution = Solutions.at(distance(Z.begin(), it));
}

bool feasible(const vector<int>& sol, const vector<vector<int>>& A, const vector<int>& b) {
    for (int i = 0; i < A.size(); i++) {
        int fes = 0;
        for (int j = 0; j < A.at(i).size(); j++) {
            fes += sol[j] * A[i][j];
        }
        if (fes > b[i]) {
            return false;
        }
    }
    return true;
}

pair<int, vector<int>> full_enum(const vector<int>& v, const vector<vector<int>>& A, const vector<int>&b) {
    int N = pow(2, v.size());
    map<int, vector<int>> result;
    for (int i = 0; i < N; i++) {
        vector<int> solution(v.size(), 0);
        int t = i;
        int j = 0;
        while (t > 0) {
            solution[j] = t % 2;
            j += 1;
            t /= 2;
        }
        if (feasible(solution, A, b)) {
            int profit  = 0;
            for (int k = 0; k < v.size(); k++) {
                profit += v[k] * solution[k];
            }
            result[profit] = solution;
        }
    }
    return { result.rbegin()->first, result.rbegin()->second };
}

void generate_sample(const string& filename, int N, int M) {   //Генерация данных
    srand(time(NULL));
    ofstream output;
    vector<int> restrictions;
    output.open(filename);
    restrictions.resize(M);
    output << N << " " << M << '\n';
    for (int i = 0; i < N; i++) {
        output << rand() % 95 + 5 << " ";
    }
    output << "\n\n";
    for (int j = 0; j < M; j++) {
        restrictions[j] = 0;
        for (int i = 0; i < N; i++) {
            int elem = rand() % 95 + 5;
            output << elem << " ";
            restrictions[j] += elem;
        }
        output << '\n';
    }
    output << "\n";
    for (auto e : restrictions) {
        output << e - rand() % (e / 2) << " ";
    }
    output.close();
}   // 

void input_and_preparation(int& N, int& M, vector<int>& v, vector<int>& b,
    vector<vector<int>>& A, vector<vector<int>>& Subscripts) {  // Ввод (или ввод+генерация) данных с проверочным выводом
    int answer = 0;
    cout << " Input: 0 - I want to work with example, 1 - I want to generate, 2 - I want to work with previously generated: ";
    cin >> answer;
    fstream input;
    if (answer == 1) {
        int n, m;
        cout << "Number of variables:";
        cin >> n;
        cout << "Number of constaints:";
        cin >> m;
        generate_sample("generated_input.txt", n, m);
        input.open("generated_input.txt");
    }
    else if (answer == 2) {
        input.open("generated_input.txt");
    }
    else {
        input.open("input.txt");
    }

    input >> N >> M;
    v.resize(N);
    for (int i = 0; i < N; i++) {
        input >> v[i];
    }

    for (int j = 0; j < M; j++) {          //Данный цикл с использованием вспомогательного map ранжирует переменные по приоритетности
        vector<int> a;
        map<float, int> order;
        a.resize(N);
        for (int i = 0; i < N; i++) {
            input >> a[i];
            float distinct = v[i] * 1.0 / a[i];
            order[distinct] = i;
        }
        A.push_back(a);
        vector<int> subscript;
        for (std::map<float, int>::reverse_iterator it = order.rbegin();
            it != order.rend(); ++it) {
            subscript.push_back(it->second);            // Отранжированные по возрастанию приоритетности переменные записываются (как вспомогательные индексы перестановок)
        }                                               // в матрицу Subscripts, где каждая строка соответсвует своему ограничению
        Subscripts.push_back(subscript);
    }

    for (int j = 0; j < M; j++) {
        int weight;
        input >> weight;
        b.push_back(weight);
    }

    input.close();
}

void output(int N, int M, const vector<int>& v, const vector<int>& b,
    const vector<vector<int>>& A, const vector<vector<int>>& Subscripts) {
    cout << "\nN, M: " << N << " " << M << endl;
    cout << "Maximisation function weights v: ";
    for (int i = 0; i < N; i++) {
        cout << v[i] << " ";
    }
    cout << "\n\nRestriction weights A:\n";

    for (int j = 0; j < M; j++) {          //Данный цикл с использованием вспомогательного map ранжирует переменные по приоритетности
        for (int i = 0; i < N; i++) {
            cout << A[j][i] << " ";
        }
        cout << endl;
    }
    cout << "\nRestrictions b: ";
    for (int j = 0; j < M; j++) {
        cout << b[j] << " ";
    }

    cout << "\n\nSubscripts: \n";
    for (int i = 0; i < Subscripts.size(); i++) {
        for (int j = 0; j < Subscripts[i].size(); j++) {
            cout << Subscripts[i][j] + 1 << " ";
        }
        cout << endl;
    }
    cout << endl;
}

Node find_max_child(vector<Node>& v) { //Поиск ноды с максимальной границей
    Node result;
    for (const auto& n : v) {
        if  (n.Bound > result.Bound) {
            result = n;
        }
    }
    return result;
}

int main()
{
    int N, M;               //N - количество переменных, M - количество ограничений
    vector<int> v, b;       //Постарался сохранить нотацию оригинала, A у меня стал матрицей M на N
    vector<vector<int>> A, Subscripts;
    input_and_preparation(N, M, v, b, A, Subscripts);           //Функция считывает (или генерирует) данные по моим лекалам (пример в input.txt)
    output(N, M, v, b, A, Subscripts);
    {
        LOG_DURATION("Shih duration");
        vector<Node> Nodes;                                         // Здесь будем хранить ноды
        Node root;                                                  //Начало расчетной части (задаем корневую ноду)
        bound_for_node(root, Subscripts, v, A, b);                  //Нода передается по ссылке и приводится к окончательному виду при завершении функции
        Nodes.push_back(root);
        int k = 0;
        while (k < 10000) {
            set<int> I_tmp = root.I, E_tmp = root.E;                //Передаем во множества граничные элементы
            I_tmp.insert(root.bound_item_index);
            E_tmp.insert(root.bound_item_index);
            Node child_left, child_right;                           //Задаем два потомка для текущего корняи вызываем для них функцию подсчёта границы
            child_left.number = Nodes.back().number + 1;
            child_right.number = Nodes.back().number + 2;
            child_left.E = E_tmp;
            child_left.I = root.I;
            child_right.I = I_tmp;
            child_right.E = root.E;
            bound_for_node(child_left, Subscripts, v, A, b);
            bound_for_node(child_right, Subscripts, v, A, b);
            Nodes.push_back(child_left);                            //Записываем полученные ноды с границами в массив
            Nodes.push_back(child_right);
            auto no_max = remove(Nodes.begin(), Nodes.end(), root);
            Nodes.erase(no_max, Nodes.end());
            Node max_child = find_max_child(Nodes);                 //Ищем ноду с максимальной границей
            k += 1;
            if (max_child.bound_item_index == -1) {                 //Если для ноды с максимальной из границ достигнуто условие выполнимости, завершаем работу
                cout << "Result at node " << max_child.number << " with value of " << max_child.Bound << "\nVector indexes for solution B&B: ";
                float s = 0.0;
                for (int i = 0; i < v.size(); i++) {
                    cout << max_child.solution[i] << " ";
                    s += v[i] * max_child.solution[i];
                }
                cout << endl;
                cout << "Just checking: " << s << "\n";
                break;
            }
            else {                                                  //Если условие не достигнуто, максимальная нода становится родителем
                root = max_child;
            }
        }
    }
    {
        LOG_DURATION("Full enumeration duration");
        pair<int, vector<int>> sol_enum = full_enum(v, A, b);
        cout << "\nFull enum solution: " << sol_enum.first << "\nVector indexes for solution full enum: ";
        for (auto c : sol_enum.second) {
            cout << c << " ";
        }
        cout << endl;
    }
}
