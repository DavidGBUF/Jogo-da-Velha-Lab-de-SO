# Nome do compilador
CXX = g++

# Flags de compilação
CXXFLAGS = -std=c++11 -pthread

# Nome do executável
TARGET = jogo_da_velha

# Nome do arquivo fonte
SRC = jogo_da_velha.cpp

# Regra padrão para compilar o programa
all: $(TARGET)

# Regra para compilar o executável
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

# Regra para limpar arquivos gerados pela compilação
clean:
	rm -f $(TARGET)

# Regra para executar o programa
run: $(TARGET)
	./$(TARGET)
