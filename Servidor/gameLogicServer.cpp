/* 
    Tapão
 * Algumas regras:
 *
 *  Todos os jogadores recebem um monte de cartas, e elas devem ser jogadas de costas, para que os valores sejam
 * descobertos apenas no momento da jogada. Cada jogador fala uma carta da ordem de ás até réi, e coloca a carta
 * do topo do monte na mesa. O jogador seguinte fala a próxima carta da sequência, ou ás caso a carta falada antes
 * seja um rei, e joga uma carta do topo do baralho. Se a carta falada for a mesma que a tirada do topo, todos os
 * jogadores precisam bater a mão no monte, e aquele que bater por último recebe a pilha de cartas jogadas. Ganha
 * os jogadores que ficarem sem nenhuma carta antes do último. 
 *
 * Número de cartas: 52
 * As cartas são divididas por igual entre os jogadores
 * 2 a 6 Jogadores
 */

#include "gameLogicServer.hpp"

#define DECK_SIZE 52 // quantidade de cartas no baralho

#define PAUS 0
#define COPAS 1
#define ESPADAS 2
#define OURO 3

#define MSG_SIZE 256


Game::~Game() {}

// Funcao que trata o baralho de acordo com o numero de players
void Game::initializePlayers(std::vector<int> playersSocket){
    int curSocket;
    bool trueVar = true;
    activePlayersNB = playersSocket.size(); //pega a quantidade de players pelo socket
    
    cardsPerPlayer =  52/activePlayersNB;   //divide o tamanho do baralho entre os player
    curDeckSize = cardsPerPlayer*activePlayersNB;   //recupera o novo tamanho do baralho jah que pode ser menor
    int  deckReduction = 52 - curDeckSize;
    for(int i = 0;i<deckReduction;i++) deck.pop_back(); //Diminui o número de cartas do deck de forma que fique um deck com cartas iguais pra cada um

    for(int i = 0; i < activePlayersNB; i++){   //para cada jogador:
        curSocket = playersSocket.at(i);
        activePlayers[curSocket] = _player{};
        activePlayers[curSocket].myTurn = !trueVar; //turno do jogador
        activePlayers[curSocket].active = trueVar;
        activePlayers[curSocket].socket = curSocket;    //socket dele
        activePlayers[curSocket].cardsInHand = cardsPerPlayer;  //tamanho do baralho do jogador
    }
    activePlayers[playersSocket.at(0)].myTurn = true;
    giveCards();
}

//Funcao que inicia os sockets de cada jogador, cria deck
Game::Game(std::unordered_map<int, int> playersSocketPair) {
    std::vector<int> playersSocket; //cria um vetor com os sockets de cada jogador
    for(auto & sockets : playersSocketPair){  //roda o vetor inserindo os sockets
        playersSocket.push_back(sockets.second);
    }
    createDeck();
    initializePlayers(playersSocket);
}

// Função que adiciona todas as cartas ao baralho (13 de cada naipe)
void Game::createDeck() {
    for (int suit = 0; suit < 4; suit++) {  //para cada naipe
        for (int j = 0; j < 13; j++) {  //todas as 13 cartas
            card addToDeck = { suit, cardsSequence[j].first };  //cria a respectiva carta
            deck.push_back(addToDeck);  //coloca no baralho
        }
    }
    printf("Deck inicializado\n");

    shuffleDeck();  //embaralha o deck
    return;
}

// Função que dá suffle no baralho
void Game::shuffleDeck() {
    // Embaralhando deck com uma seed que usa o tempo exato de agora
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

    printf("Begin: %c, End: %c\n", deck.front().value, deck.back().value);
    std::shuffle(deck.begin(), deck.end(), std::default_random_engine(seed));

    printf("Deck embaralhado\n");
    return;
}

// Função que distribui as cartas aos jogadores
void Game::giveCards() {
    // Distribui as cartas, dando a mesma quantidade para cada jogador
    for (auto & personSocket : activePlayers){
        playersSequence.push_back(personSocket.first);
        std::cout << "Player with socket " << personSocket.first << std::endl;
        for(int i=0;i<cardsPerPlayer;i++){  //roda o baralho de cada jogador
            std::cout << "\tCard:("<< getCardName(deck.back()) << ")" << std::endl;
            personSocket.second.deck.push_back(deck.back());    //retira do baralho de 52 cartas e divide entre pequenos baralhos aos jogadores
            deck.pop_back();    //retira as cartas da pilha do primeiro deck
        }
    }
    std::sort(playersSequence.begin(), playersSequence.end());

    printf("Cartas distribuidas\n");
    return;
}

// Função com o estado de colocar uma carta no monte
std::unordered_map<int,std::string> Game::cardPlayed(int personId){
    // Variável que diz se colocou uma carta em cima da carta correta da sequência
    bool playedOnTopOfRightCard =  (stack.size() != 0) && (stack.front().value == cardsSequence[desiredCardId].first);
    
    desiredCardId = (desiredCardId+1)%(cardsSequence.size());   //pega o id da proxima carta desejada
    topCard = activePlayers[personId].deck.front(); //coloca a carta do jogador na mesa
    std::cout << "Card Played: " << getCardName(topCard) << std::endl;
    
    activePlayers[personId].deck.pop_front();   //tira a carta do baralho do jogador

    stack.push_front(topCard);  //coloca no baralho da mesa


    activePlayers[personId].cardsInHand --; //diminui a quantidade de carta na mao do jogador
    activePlayers[personId].myTurn = false; //o jogador da vez passa o turno
    curPlayerIndex = (curPlayerIndex+1)%activePlayersNB;
    activePlayers[playersSequence.at(curPlayerIndex)].myTurn = true;    //o proximo comeca o turno
    tapQtty = 0;
    
    // Se o jogador de fato jogou em cima de uma carta certa
    if(playedOnTopOfRightCard) makePersonGainCards(personId);

    // Retorna mensagens diferentes se acabaram as cartas do jogador ou não
    return (activePlayers[personId].cardsInHand == 0 ? sendEndGameMessage(personId) : getAllClientsMessages()); //envia a mensagem aos clientes conectados
}

//quando a carta da sequencia foi a mesma que a carta jogada o jogo chega no tapao
bool Game::willGainCards(){
    bool isCorrectCard = (cardsSequence[desiredCardId].first == topCard.value); //se a carta da sequencia for igual a jogada
    std::cout << "A carta da sequência é a mesma do topo da pilha\n";
    
    // Retorna se ele foi o último a bater na carta certa ou o último a bater na carta errada 
    return isCorrectCard ? (++tapQtty == activePlayersNB) : (tapQtty++ == 0);   //incrementa o valor ateh todos os jogadores terem clicado
}

// Função que dá todas as cartas da pilha para o jogador que perdeu
void Game::makePersonGainCards(int personId){
    activePlayers[personId].cardsInHand += stack.size();    //aumenta o int que indica o tam do baralho do jogador

    while(!stack.empty()){  //enquanto o baralho da mesa nao acabar
        activePlayers[personId].deck.push_back(stack.back());   //coloca as cartas no baralho do jogador
        stack.pop_back();   // tira do baralho da mesa
    }
}


// Faz todo o processamento de quando o jogador dá um tapão no monte, usando as funções anteriores
std::unordered_map<int,std::string> Game::cardTapped(int personId){
    if(willGainCards()) makePersonGainCards(personId);

    return getAllClientsMessages();
}

// Converte o valor da carta em string
std::string Game::getCardName(_card c){
    std::string resp(1, c.value);
    resp += "_" + suitIdToStr[c.suit];
    return resp;
}

//cria a mensagem para o cliente informando o decorrer do jogo
std::string Game::getClientMessage(int personId){
    auto person = activePlayers[personId];
    std::string nextCardName = getCardName(topCard);
    std::string desiredCardStr(1, cardsSequence[desiredCardId].first); 
    std::string message = "desiredCardID#" + desiredCardStr + "|nextCardName#"+ nextCardName + "|nextActivePlayerID#" + std::to_string(curPlayerIndex) + "|nbCardsInHand#";
    //cria a mensagem para o cliente com as informacoes de proxima carta, jogador da vez, e cartas de cada jogador
    // nbCardsInHand#10,3,4,5,6
    for(int i = 0; i < activePlayersNB; i++) {
        message += std::to_string(activePlayers[playersSequence[i]].cardsInHand);   //para cada jogador cria a sequencia de cartas na mao dele em string
        message += ( i < activePlayersNB - 1 ? "," : "");
    }
    
    message += "|"; //a mensagem deve acabar em |
    std::cout << "My id:" << personId << ";" << message << std::endl;   //mostrando a mensagem enviada no programa do servidor
    return message;
}

//cria a mensagem do getClientMessage para todos os jogadores
std::unordered_map<int,std::string> Game::getAllClientsMessages(){
    std::unordered_map<int,std::string> allMessages;    //cria um map de mensagens a serem enviadas
    int socketId;
    for(int i=0;i<activePlayersNB;i++){ //roda para cada player:
        socketId = playersSequence.at(i);   //captura o socket de cada jogador
        allMessages[socketId] = getClientMessage(socketId); //coloca no map o socket de cada jogador e a mensagem do cliente
    }
    return allMessages;
}

//cria a mensagem de fim de jogo e envia.
std::unordered_map<int, std::string> Game::sendEndGameMessage(int personId) {
    std::unordered_map<int,std::string> allMessages;
    std::string endGameMessage = "endGame#Fim de jogo! O vencedor é o jogador de ID: " + std::string(1, personId);   //cria a mensagem de vencedor
    int socketId;
    for(int i=0;i<activePlayersNB;i++){ //envia a todos os jogadores
        socketId = playersSequence.at(i);   //captura o socket do jogador a ser enviado
        allMessages[socketId] = endGameMessage;
    }

    return allMessages;
}