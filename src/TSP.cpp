#include "TSP.h"

TSP::TSP(double ***mPointer, int dimension){
    matrix = *mPointer;
    this->dimension = dimension;
    cost = 0;

    int i;

    candidateList.resize(dimension);
    //Preenchendo a lista de candidatos
    for(i = 0; i < dimension; i++)
        candidateList[i] = i;
    
    //Criando um subtour inicial
    subtour();

    //Preenchendo o vetor solucao viavelmente
    initialRoute();
    
    //RVND
    std::vector<char> neighborList(NEIGHBORLIST_SIZE);
    for(i = 0; i < NEIGHBORLIST_SIZE; i++)
        neighborList[i] = i+1;

    while(!neighborList.empty()){
        i = random(neighborList.size())-1;
        switch(neighborList[i]){
            case 1:
                if(!swap())
                    neighborList.erase(neighborList.begin() + i);
                break;

            case 2:
                if(!revert())
                    neighborList.erase(neighborList.begin() + i);
                break;

            case 3:
                if(!reinsert(1))
                    neighborList.erase(neighborList.begin() + i);
                break;

            case 4:
                if(!reinsert(2))
                    neighborList.erase(neighborList.begin() + i);
                break;

            case 5:
                if(!reinsert(3))
                    neighborList.erase(neighborList.begin() + i);
                break;
                
        }
    }
}

int TSP::random(int num){
    static bool seedGenerated = false;

    if(!seedGenerated){
        srand(time(NULL));
        seedGenerated = true;
    }

    return (rand()%num)+1;
}

void TSP::subtour(){
    //Obtendo um item incial aleatorio
    int first = random(dimension);

    //Inserindo-o na solucao e removendo-o da lista de candidatos
    route.push_back(first);
    candidateList.erase(candidateList.begin() + first);

    //Inserindo itens aleatorios da lista de candidatos na solucao
    for(int i = 0; i < SUBTOUR_SIZE; i++){
        int j = random(candidateList.size()) - 1;
        cost += matrix[route[i]][candidateList[j]];
        route.push_back(candidateList[j]);
        candidateList.erase(candidateList.begin() + j);  
    }

    //Terminando o ciclo hamiltoniano
    route.push_back(first);
    cost += matrix[route[SUBTOUR_SIZE]][route[SUBTOUR_SIZE+1]];
}

void TSP::initialRoute(){
    tMove* nextNode;
    
    //Repetindo ate obter uma solucao inicial viavel
    while (!candidateList.empty()){
        //Calculando o custo de insercao de cada um dos candidatos restantes
        std::vector <tMove*> costVector((route.size()-2) * candidateList.size());

        for(int i = 1, k = 0; i < route.size()-1; i++){
            for(int j = 0; j < candidateList.size(); j++){
                //Alocando um no de custo
                nextNode = (tMove*)malloc(sizeof(tMove));

                //Atribuindo os indices aos seus elementos
                nextNode->i = j;
                nextNode->j = i;

                //Calculando o custo de insercao e o inserindo no vetor
                nextNode->delta =  matrix[route[i]][candidateList[j]] 
                                  +matrix[candidateList[j]][route[i+1]]
                                  -matrix[route[i]][route[i+1]];

                costVector[k++] = nextNode;
            }
        }
    
        //Organizando o vetor de custo
        std::sort(costVector.begin(), costVector.end());
        
        //Obtendo um item em um intervalo aleatorio do vetor de custo
        nextNode = costVector[random((int)random(10)/10.0 * (costVector.size()- 1))];

        //Inserindo o item na solucao, e o removendo do vetor de candidatos
        route.insert(route.begin() + (nextNode->j) + 1, candidateList[nextNode->i]);
        cost += nextNode->delta;
        candidateList.erase(candidateList.begin() + nextNode->i);
    }
}

bool TSP::swap(){
    //Criando um movimento de swap com delta infito, a fim de nao pegar lixo de memoria, e nao bugar o if
    tMove bestSwap = {0, 0, INFINITY};
    double delta;

    //Repetindo ate encontrar o swap com menor delta
    for(int i = 1; i < dimension - 2; i++){
        for(int j = i + 1; j < dimension - 1; j++){
            if(j-i != 1)
                delta =  matrix[route[i]][route[j-1]]
                        +matrix[route[i]][route[j+1]]
                        +matrix[route[j]][route[i-1]]
                        +matrix[route[j]][route[i+1]]
                        -matrix[route[i]][route[i-1]]
                        -matrix[route[i]][route[i+1]]
                        -matrix[route[j]][route[j-1]]
                        -matrix[route[j]][route[j+1]];
            else
                delta =  matrix[route[i]][route[j+1]]
                        +matrix[route[j]][route[i-1]]
                        -matrix[route[i]][route[i-1]]
                        -matrix[route[j]][route[j+1]];
                 
            
            if(bestSwap.delta > delta){
                bestSwap.i = i;
                bestSwap.j = j;
                bestSwap.delta = delta;
            }
        }
    }

    //Realizando o swap na rota, e inserindo o delta no custo
    if(bestSwap.delta < 0){
        cost = cost + bestSwap.delta;
        std::swap(route[bestSwap.i], route[bestSwap.j]);
        return true;
    }

    return false;
}

bool TSP::revert(){
    tMove bestReversion = {0, 0, INFINITY};
    double delta;

    for(int i = 1; i < dimension - 2; i++){
        for(int j = i + 2; j < dimension - 1; j++){
            delta =  matrix[route[i]][route[j+1]]
                    +matrix[route[j]][route[i-1]]
                    -matrix[route[i]][route[i-1]]
                    -matrix[route[j]][route[j+1]];
            
            if(bestReversion.delta > delta){
                bestReversion.i = i;
                bestReversion.j = j;
                bestReversion.delta = delta;
            }
        }
    } 

    if(bestReversion.delta < 0){
        cost = cost + bestReversion.delta;
        std::reverse(route.begin() + bestReversion.i, route.begin() + bestReversion.j+1);
        return true;
    }

    return false;
}

bool TSP::reinsert(int num){
    tMove bestReinsertion = {0, 0, INFINITY};
    double delta;

    for(int i = 1; i < dimension - (num-1); i++){
        for(int j = 0; j < dimension - num; j++){ //O j comeca em zero pois o i sera inserido numa posicao a frente do j
            //Checando se ele esta dentro do intervalo que sera movimentado
            if(j+1 == i){
                j += num;
                continue;
            }

            delta =  matrix[route[j]][route[i]]
                    +matrix[route[i+(num-1)]][route[j+1]]
                    +matrix[route[i-1]][route[i+num]]
                    -matrix[route[i-1]][route[i]]
                    -matrix[route[i+(num-1)]][route[i+num]]
                    -matrix[route[j]][route[j+1]];
            
            if(bestReinsertion.delta > delta){
                bestReinsertion.i = i;
                bestReinsertion.j = j;
                bestReinsertion.delta = delta;
            }
        }
    }
    
    if(bestReinsertion.delta < 0){
        cost = cost + bestReinsertion.delta;

        std::vector<int> subroute(route.begin() + bestReinsertion.i, route.begin() + bestReinsertion.i + num);
        route.erase(route.begin() + bestReinsertion.i, route.begin() + bestReinsertion.i + num);

        if(bestReinsertion.i > bestReinsertion.j)
            route.insert(route.begin() + (bestReinsertion.j+1), subroute.begin(), subroute.end());
        else
            route.insert(route.begin() + bestReinsertion.j-(num-1), subroute.begin(), subroute.end());
        
        return true;
    }

    return false;
}

void TSP::printSolution(){
    for(int i = 0; i < route.size(); i++)
        printf("%d%s", route[i]+1, i+1 == route.size()?"\n":", ");
}

double TSP::getCost(){
    return cost;
}

double TSP::getRealCost(){
    double sum = 0;

    for(int i = 0; i < dimension; i++){
        sum+= matrix[route[i]][route[i+1]];
    }
    
    return sum;
}

void TSP::printMatrix(){
    printf("Dimension: %d\n", dimension);
    for(int i = 0; i < dimension; i++){
        for(int j = 0; j < dimension; j++){
            char endian = ((j+1)==dimension) ? '\n' : ' ';
            std::cout <<  matrix[i][j] << endian;
        }
    }
}