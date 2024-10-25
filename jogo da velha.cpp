#include <iostream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

// Mutex e variável de condição para sincronizar o turno dos jogadores
mutex mtx;
condition_variable cv;
bool turnoJogador1 = true; // Controle de turno: true para jogador 1, false para jogador 2
bool fimJogo = false;      // Flag para indicar o término do jogo

// Função para limpar a tela
void limparTela() {
    system("clear");  // Usa o comando 'clear' do terminal para limpar a tela
}

// Inicializa o tabuleiro com '-'
void preencherMatriz(char tabuleiro[3][3]) {
    for (int linha = 0; linha < 3; linha++)
        for (int coluna = 0; coluna < 3; coluna++)
            tabuleiro[linha][coluna] = '-';
}

// Exibe o tabuleiro de forma estruturada
void exibirTabuleiro(const char tabuleiro[3][3]) {
    cout << "\n";
    for (int linha = 0; linha < 3; linha++) {
        for (int coluna = 0; coluna < 3; coluna++) {
            cout << tabuleiro[linha][coluna];
            if (coluna < 2) cout << " | ";  // Separador de colunas
        }
        if (linha < 2) cout << "\n-----------\n";  // Separador de linhas
    }
    cout << "\n";
}

// Verifica se há um vencedor no tabuleiro e retorna o jogador vencedor (1 ou 2)
// Retorna 0 caso não haja vencedor
int conferirTabuleiro(const char tabuleiro[3][3]) {
    // Verifica linhas e colunas para identificar um vencedor
    for (int i = 0; i < 3; i++) {
        if (tabuleiro[i][0] == tabuleiro[i][1] && tabuleiro[i][1] == tabuleiro[i][2] && tabuleiro[i][0] != '-')
            return (tabuleiro[i][0] == 'X') ? 1 : 2;
        if (tabuleiro[0][i] == tabuleiro[1][i] && tabuleiro[1][i] == tabuleiro[2][i] && tabuleiro[0][i] != '-')
            return (tabuleiro[0][i] == 'X') ? 1 : 2;
    }
    // Verifica diagonais
    if (tabuleiro[0][0] == tabuleiro[1][1] && tabuleiro[1][1] == tabuleiro[2][2] && tabuleiro[0][0] != '-')
        return (tabuleiro[0][0] == 'X') ? 1 : 2;
    if (tabuleiro[0][2] == tabuleiro[1][1] && tabuleiro[1][1] == tabuleiro[2][0] && tabuleiro[0][2] != '-')
        return (tabuleiro[0][2] == 'X') ? 1 : 2;

    return 0;  // Nenhum vencedor
}

// Exibe um mapa com instruções para orientar a escolha de posições
void exibirMapaInstrucoes() {
    cout << "\nMAPA DE INSTRUCOES\n 7 | 8 | 9\n-----------\n 4 | 5 | 6\n-----------\n 1 | 2 | 3\n";
}

// Função de jogada, executada em uma thread para cada jogador
void jogar(char simbolo, string nome, char tabuleiro[3][3], int posicoes[9][2], bool &ganhou) {
    while (!fimJogo) {
        unique_lock<mutex> lock(mtx);
        // Espera o turno do jogador e que o jogo não tenha terminado
        cv.wait(lock, [=] { return (simbolo == 'X' ? turnoJogador1 : !turnoJogador1) && !fimJogo; });

        if (fimJogo) break;  // Sai do loop se o jogo já terminou

        limparTela();
        exibirTabuleiro(tabuleiro);
        exibirMapaInstrucoes();

        int posicao, linha, coluna;
        bool posicaoValida = false;
        string mensagem = "";

        // Loop para garantir que o jogador insira uma posição válida
        while (!posicaoValida) {
            cout << mensagem << nome << ", digite uma posicao conforme o mapa (1-9): ";
            cin >> posicao;

            // Valida se a posição está no intervalo correto
            if (posicao < 1 || posicao > 9) {
                mensagem = "\nPosicao invalida! Tente novamente.\n";
                continue;
            }

            // Converte a posição escolhida para índices de linha e coluna no tabuleiro
            linha = posicoes[posicao - 1][0];
            coluna = posicoes[posicao - 1][1];

            // Verifica se a posição está livre
            if (tabuleiro[linha][coluna] == '-') {
                tabuleiro[linha][coluna] = simbolo;  // Marca o tabuleiro
                posicaoValida = true;
            } else {
                mensagem = "\nEssa posicao ja foi jogada!\n";
            }
        }

        // Verifica se o jogador venceu após sua jogada
        ganhou = (conferirTabuleiro(tabuleiro) == (simbolo == 'X' ? 1 : 2));
        if (ganhou) {
            fimJogo = true;  // Marca o jogo como terminado
            limparTela();
            exibirTabuleiro(tabuleiro);
            cout << "O jogador " << nome << " venceu!!!\n";
            cout << "Encerrando o jogo. Até mais!\n";
            exit(0);  // Encerra o programa imediatamente após a vitória
        }

        turnoJogador1 = !turnoJogador1;  // Alterna o turno para o próximo jogador
        cv.notify_all();  // Notifica todas as threads para a mudança de turno
    }
}

// Função principal do jogo, controla as threads dos jogadores e verifica o estado do jogo
void jogo(string nome1, string nome2) {
    char tabuleiro[3][3];
    int posicoes[9][2] = {{2, 0}, {2, 1}, {2, 2}, {1, 0}, {1, 1}, {1, 2}, {0, 0}, {0, 1}, {0, 2}};
    bool ganhou1 = false, ganhou2 = false;

    preencherMatriz(tabuleiro);  // Inicializa o tabuleiro
    fimJogo = false;             // Reseta o estado do jogo

    // Cria threads para cada jogador
    thread jogador1(jogar, 'X', nome1, ref(tabuleiro), posicoes, ref(ganhou1));
    thread jogador2(jogar, 'O', nome2, ref(tabuleiro), posicoes, ref(ganhou2));

    jogador1.join();  // Aguarda o término da thread do jogador 1
    jogador2.join();  // Aguarda o término da thread do jogador 2

    // Verifica se o jogo terminou em empate
    if (!ganhou1 && !ganhou2) {
        limparTela();
        exibirTabuleiro(tabuleiro);
        cout << "\nEMPATE!!!\n";
        cout << "Encerrando o jogo. Até mais!\n";
        exit(0);  // Encerra o programa em caso de empate
    }
}

// Função que exibe o menu principal do jogo
void menu() {
    string nome1, nome2;
    int opcao = 0;

    while (opcao != 3) {
        limparTela();
        cout << "\nJOGO DA VELHA!!!\n1 - Jogar\n2 - Sobre o jogo\n3 - Sair\nSua opcao: ";
        cin >> opcao;

        switch (opcao) {
            case 1:
                cout << "\nNome do jogador 1: ";
                cin.ignore();  // Limpa o buffer de entrada
                getline(cin, nome1);
                cout << "Nome do jogador 2: ";
                getline(cin, nome2);
                jogo(nome1, nome2);  // Inicia o jogo com os nomes dos jogadores
                break;
            case 2:
                limparTela();
                cout << "\nEste jogo foi desenvolvido com o uso de threads e semáforos em 2024.\n1 - Voltar\n2 - Sair\nDigite: ";
                cin >> opcao;
                opcao = (opcao == 1) ? 0 : 3;  // Atualiza a opção conforme a escolha do usuário
                break;
            case 3:
                cout << "\nAte mais!!!\n";
                break;
            default:
                cout << "\nOpcao Invalida, tente novamente.\n";
                this_thread::sleep_for(chrono::seconds(2));
        }
    }
}

// Função principal que inicializa o menu
int main() {
    srand((unsigned)time(NULL));  // Semente para geração aleatória
    menu();  // Chama o menu principal
    return 0;
}
