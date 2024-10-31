#include <iostream>
#include <string>
#include <cstdlib>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

// Mutex e variável de condição para sincronizar o turno dos jogadores
mutex mtx;                      // Mutex para proteger o acesso a variáveis compartilhadas
condition_variable cv;          // Variável de condição para coordenar o turno dos jogadores
bool turnoJogador1 = true;      // Controle de turno: true para jogador 1, false para jogador 2
bool fimJogo = false;           // Flag para indicar o término do jogo

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
    for (int i = 0; i < 3; i++) {
        if (tabuleiro[i][0] == tabuleiro[i][1] && tabuleiro[i][1] == tabuleiro[i][2] && tabuleiro[i][0] != '-')
            return (tabuleiro[i][0] == 'X') ? 1 : 2;
        if (tabuleiro[0][i] == tabuleiro[1][i] && tabuleiro[1][i] == tabuleiro[2][i] && tabuleiro[0][i] != '-')
            return (tabuleiro[0][i] == 'X') ? 1 : 2;
    }
    if (tabuleiro[0][0] == tabuleiro[1][1] && tabuleiro[1][1] == tabuleiro[2][2] && tabuleiro[0][0] != '-')
        return (tabuleiro[0][0] == 'X') ? 1 : 2;
    if (tabuleiro[0][2] == tabuleiro[1][1] && tabuleiro[1][1] == tabuleiro[2][0] && tabuleiro[0][2] != '-')
        return (tabuleiro[0][2] == 'X') ? 1 : 2;

    return 0;  // Nenhum vencedor
}

// Verifica se o tabuleiro está cheio e não há vencedor, indicando um empate
bool verificarEmpate(const char tabuleiro[3][3]) {
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (tabuleiro[i][j] == '-') {
                return false; // Há posições vazias, então ainda não é empate
            }
        }
    }
    return true; // Todas as posições estão preenchidas
}

// Exibe um mapa com instruções para orientar a escolha de posições
void exibirMapaInstrucoes() {
    cout << "\nMAPA DE INSTRUCOES\n 7 | 8 | 9\n-----------\n 4 | 5 | 6\n-----------\n 1 | 2 | 3\n";
}

// Função de jogada, executada em uma thread para cada jogador
void jogar(char simbolo, string nome, char tabuleiro[3][3], int posicoes[9][2], bool &ganhou) {
    while (!fimJogo) {
        // Bloqueia o mutex para garantir acesso exclusivo às variáveis compartilhadas
        unique_lock<mutex> lock(mtx);
        
        // Espera até que seja a vez deste jogador jogar ou que o jogo termine
        cv.wait(lock, [simbolo] { 
            return (simbolo == 'X' && turnoJogador1) || (simbolo == 'O' && !turnoJogador1); 
        });

        if (fimJogo) return;  // Sai do loop se o jogo já terminou

        // Exibe o tabuleiro atualizado e o mapa de instruções
        limparTela();
        exibirTabuleiro(tabuleiro);
        exibirMapaInstrucoes();

        int posicao, linha, coluna;
        bool posicaoValida = false;
        string mensagem = "";

        // Laço para validar a posição escolhida pelo jogador
        while (!posicaoValida) {
            cout << mensagem << nome << ", digite uma posicao conforme o mapa (1-9): ";
            cin >> posicao;

            if (posicao < 1 || posicao > 9) {
                mensagem = "\nPosicao invalida! Tente novamente.\n";
                continue;
            }

            linha = posicoes[posicao - 1][0];
            coluna = posicoes[posicao - 1][1];

            // Verifica se a posição está disponível
            if (tabuleiro[linha][coluna] == '-') {
                tabuleiro[linha][coluna] = simbolo;
                posicaoValida = true;
            } else {
                mensagem = "\nEssa posicao ja foi jogada!\n";
            }
        }

        // Checa se o jogador atual venceu após a jogada
        ganhou = (conferirTabuleiro(tabuleiro) == (simbolo == 'X' ? 1 : 2));
        if (ganhou) {
            fimJogo = true;  // Indica que o jogo terminou
            limparTela();
            exibirTabuleiro(tabuleiro);
            cout << "O jogador " << nome << " venceu!!!\n";
            cout << "Encerrando o jogo. Até mais!\n";
        }

        // Verificação de empate após a jogada
        if (verificarEmpate(tabuleiro)) {
            fimJogo = true;
            limparTela();
            exibirTabuleiro(tabuleiro);
            cout << "\nEMPATE!!!\n";
            cout << "Encerrando o jogo. Até mais!\n";
        }

        // Alterna o turno para o próximo jogador
        turnoJogador1 = !turnoJogador1;
        cv.notify_all();  // Notifica todas as threads para reavaliar a condição de turno
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
}

// Função principal que inicializa o menu
int main() {
    string nome1, nome2;
    cout << "\nNome do jogador 1: ";
    // cin.ignore();
    getline(cin, nome1);
    cout << "Nome do jogador 2: ";
    getline(cin, nome2);
    jogo(nome1, nome2);  // Inicia o jogo com os nomes dos jogadores
    return 0;
}
