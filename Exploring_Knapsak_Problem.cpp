#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <fstream>
#include <algorithm>
#include "profile.h"
#include "test_runner.h"

struct Node {
    int number = 1;
    float Bound = 0;
    int bound_item_index = 0;
    bool is_parent = 0;
    set<int> I, E;
};

bool operator == (const Node& lhs, const Node& rhs) {
    return lhs.number == rhs.number;
}

void bound_for_node(Node& node, const vector<vector<int>>& Subscripts, 
                const vector<int>& v, const vector<vector<int>>& A, const vector<int>& b) {
    vector<float> Z;            //Задаём вектора Z и bound_items - в них будут отходить границы и граничные элементы каждой операции
    vector<int> bound_items;
    for (int j = 0; j < b.size(); j++) {        
        float sum = 0;
        float z = 0;
        for (auto c : node.I) {             //Заранее подсчитываем влияние на сумму и границу фиксированных в 1 элементов
            sum += A[j][c];
            z += v[c];
        }
        if (sum > b[j]) {                   //Если эта сумма сама по себе нарушает ограничение, отбрасываем ветвь
            node.Bound = -9999;
            node.bound_item_index = 0;
            return;
        }
        int i = 0;
        float coef = 0;
        while (sum < b[j]) {
            if ((node.I.find(Subscripts[j][i]) == node.I.end()) && (node.E.find(Subscripts[j][i]) == node.E.end())) {   //Если элемент не из фиксированных множеств, работаем с ним
                if (sum + A[j][Subscripts[j][i]] < b[j]) {          //Если сумма с элементом меньше ограничения, прибавляем
                    //cout << "i: " << i << " sum: " << sum << " New weight for z: " << v[Subscripts[j][i]] <<" Z after: " <<z + v[Subscripts[j][i]]<< endl;
                    sum += A[j][Subscripts[j][i]];
                    z += v[Subscripts[j][i]];
                    if (i+1 == v.size() - node.E.size() - node.I.size()) {      //Если на этом элементе закончились нефиксированные, мы нашли границу
                        bound_items.push_back(-1); // Показатель достижимости будет -1 (проходим по ограничению, так как кончились элементы)
                        break;
                    }
                    i += 1;
                }
                else if (sum + A[j][Subscripts[j][i]] == b[j]) { //В случае равенства мы получили достижимую границу, отмечаем это
                    //cout <<j <<" "<< i << " " << Subscripts[j][i] + 1 <<" "<< sum + A[j][Subscripts[j][i]] <<" "<< b[j] << " " << v[Subscripts[j][i]] << " z =  " << z + v[Subscripts[j][i]] << endl;
                    z += v[Subscripts[j][i]];
                    bound_items.push_back(-1);  // Показатель достижимости (проходим по ограничению, так как в точности достигли раницы)
                    break;
                }
                else {
                    coef = (b[j] - sum) / A[j][Subscripts[j][i]];       //Находим коэффициент для границы из ограничения
                    //cout << "coef = " << coef << " " <<" v[sj][i] =  "<< v[Subscripts[j][i]] <<  " z += " << v[Subscripts[j][i]] * coef << " z =  "<< z + v[Subscripts[j][i]] * coef << endl;
                    //cout << endl;
                    bound_items.push_back(Subscripts[j][i]);            //Добавляем граничный элемент и его вес с коэффицентом к оцениваемой функции
                    z += v[Subscripts[j][i]] * coef;
                    break;
                }
            }
            else {
                i += 1;
                if (i >= v.size() - node.E.size() - node.I.size()) {    //Если i достигла максимального значения по размерности нефиксированных элементов, то это так же говорит о достижимой границе
                    bound_items.push_back(-1); // Показатель достижимости (проходим по ограничению, так как кончились элементы)
                    break;
                }
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
}

void generate_sample(const string& filename) {   //Генерация данных
    srand(time(NULL));
    ofstream output;
    vector<int> restrictions;
    output.open(filename);
    int N = rand() % 20 + 5;
    int M = rand() % (N / 5) + 2;
    restrictions.resize(M);
    output << N << " " << M << '\n';
    for (int i = 0; i < N; i++) {
        output << rand() % 95 + 5 << " ";
    }
    for (int j = 0; j < M; j++) {
        restrictions[j] = 0;
        for (int i = 0; i < N; i++) {
            int elem = rand() % 95 + 5;
            output << elem << " ";
            restrictions[j] += elem;
        }
        output << '\n';
    }
    for (auto e : restrictions) {
        output << e - rand() % (e / 2)<< " ";
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
        generate_sample("generated_input.txt");
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
    cout <<"\nN, M: " << N << " " << M << endl;
    cout << "Maximisation function weights v: ";
    for (int i = 0; i < N; i++) {
        cout << v[i] << " ";
    }
    cout << "\n\nRestriction weights A:\n";

    for (int j = 0; j < M; j++) {          //Данный цикл с использованием вспомогательного map ранжирует переменные по приоритетности
        for (int i = 0; i < N; i++) {
            cout << A[j][i]<<" ";
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
        if ((n.is_parent == false) && (n.Bound > result.Bound)) {
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
    vector<Node> Nodes;                                         // Здесь будем хранить ноды

    Node root;                                                  //Начало расчетной части (задаем корневую ноду)
    bound_for_node(root, Subscripts, v, A, b);                  //Нода передается по ссылке и приводится к окончательному виду при завершении функции
    root.is_parent = true;                                      //Есть ли у ноды потомки (у корня их точно два в нетривиальном случае)
    Nodes.push_back(root);
    int k = 0;
    while (k<10000) {
        set<int> I_tmp = root.I, E_tmp = root.E;                //Передаем во множества граничные элементы
        I_tmp.insert(root.bound_item_index);
        E_tmp.insert(root.bound_item_index);
        Node child_left, child_right;                           //Задаем два потомка для текущего корняи вызываем для них функцию подсчёта границы
        child_left.number = Nodes[Nodes.size()-1].number + 1;
        child_right.number = Nodes[Nodes.size()-1].number + 2;
        child_left.E = E_tmp;
        child_right.I = I_tmp;
        bound_for_node(child_left, Subscripts, v, A, b);
        bound_for_node(child_right, Subscripts, v, A, b);
        Nodes.push_back(child_left);                            //Записываем полученные ноды с границами в массив
        Nodes.push_back(child_right);
        //cout << "Current state of B: ";
        /*for (auto& n : Nodes){
            cout << n.Bound <<"[" << n.number << "]" << "(" << n.is_parent << ") " ;
        }
        cout << endl;*/
        Node max_child = find_max_child(Nodes);                 //Ищем ноду с максимальной границей
       /* if (k >= 9950) {
            cout << "I values of iteration: ";
            for (auto i : I_tmp) {
                cout << i << " ";
            }
            cout << endl;
            cout <<k<<": max_child bound "<<  max_child.Bound << endl;
        } 
        cout << k << " ";*/
        k += 1;
        if (max_child.bound_item_index == -1) {                 //Если для ноды с максимальной из границ достигнуто условие выполнимости, завершаем работу
            cout << "Result at node " << max_child.number << " with value of " << max_child.Bound;
            break;
        }
        else {                                                  //Если условие не достигнуто, максимальная нода становится родителем
            max_child.is_parent = true;
            root = max_child;
            for (auto& node: Nodes) {
                if (node == root) {
                    node.is_parent = true;
                    break;
                }
            }
        }
    }
}
