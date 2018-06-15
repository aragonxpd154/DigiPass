/*
********************************************************************************************************
  Arquivo: DIGIPASS
  Micro-processador: ATMel/Arduino Uno/Arduino Mega e suas variantes
  Linguagem: Wiring/C
  Objetivo: Digitar determinada senha para liberar o acesso

  Funcionamento: Ao ligar, realizar a leitura do status do sensor de trava, encontrado na porta;
           se o sensor estiver dando continuidade e caso o usuario não tenha digitado a;
          senha correta o alarme soa até que o usuario digite a senha correta, caso a
          senha esteja incorreta enviar dados da senha para o cliente via rede o local
          e a hora do incidente. Se a senha estiver correta o led verde acente liberando
          o acesso e os dados via rede.

  Autor: Marcos Silva
  Data: 15/02/2018
  Local: Bento Ferreira - ES

********************************************************************************************************
  Este codigo estará disponivel ao domínio público.
*/

// As mensagens do display serão adicionada caso o operante deseje adicionar na arquitetura do projeto

// Incluindo bibliotecas
#include <Wire.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // declara LCD, endereço e tipo

// Declarando as varíaveis globais e temporarias
int count = 0;

// Declarando as senhas paralelas
char pass [4] = {'1', '2', '3', '4'}; // TV Gazeta
//char pass1 [4] = {'4', '3', '2', '1'}; // TV Vitoria

//const int sirenePin = 9; // Receber o sinal em 10 ou ambos paralelo
//const int sensorPin = 11; // Receber o sinal em 12 ou ambos paralelo
const int yellowPin = 11;
const int redPin = 12;
const int greenPin = 10;
const int audioPin = 9;
const int duration = 200;
const byte LINHAS = 4; // Quatro linhas
const byte COLUNAS = 3; // Três colunas
char keys[LINHAS][COLUNAS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};// definição do layout do teclado
byte linhasPins[LINHAS] = {5, 4, 3, 2}; // pinagem para as linhas do teclado
byte colunasPins[COLUNAS] = {6, 7, 8}; // pinagem para as colunas do teclado
// mapeamento do teclado
Keypad keypad = Keypad( makeKeymap(keys), linhasPins, colunasPins, LINHAS, COLUNAS );

void setup() {
  lcd.init(); // inicializa o LCD
  lcd.backlight(); // com backlight
  Serial.begin(9600); // inicializa serial
  // modo dos pinos de audio e LEDs
  pinMode(audioPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  lcd.clear();// limpa LCD
  key_init();// inicializa processo
}

void loop() {
  char key = keypad.getKey(); // obtém informação do teclado
  if (key != NO_KEY) { // se foi teclado algo
    if (key == '#') {// se foi teclado #
      code_entry_init(); // mensagem, som e LED
      int entrada = 0; // variável de apoio; números de entradas feitas via teclado
      while (count < 4 ) { // enquanto não entrou os 4 números necessários para a senha
        char key = keypad.getKey(); // obtém informação do teclado
        if (key != NO_KEY) { // se foi teclado algo
          entrada += 1; // aumenta contrador de entradas
          tone(audioPin, 1080, 100); // sinal audível
          delay(duration);
          noTone(audioPin);
          if (key == pass[count])count += 1; // verifica na sequencia da senha, se correto aumenta contador
          if ( count == 4 ) {
            unlocked(); // chegou a 4 digitos corretos, libera acesso
          }
          if ((key == '#') || (entrada == 4)) { // foi teclado # ou 4 entradas incorretas
            key_init();// inicializa
            break;// interrompe loop
          }
        }
      }
    }
    if (key == '*') { // se foi teclado *
      old_pass_check();// mensagem para entrar a senha antiga
      int entrada = 0;
      while (count < 4 ) {
        char key = keypad.getKey();
        if (key != NO_KEY) {
          entrada += 1;
          tone(audioPin, 1080, 100);
          delay(duration);
          noTone(audioPin);
          if (key == pass[count])count += 1;
          if ( count == 4 ) { // foi teclado a senha antiga corretamente
            get_new_pass();// chama função para entrada da nova senha
          }
          if ((key == '*') || (entrada == 4)) { // foi teclado * ou entrou 4 números errados
            key_init();// inicializa
            break; // interrompe loop
          }
        }
      }
    }
  }
}

void locked() {
  lcd.clear();
  lcd.print("    ACESSO,");
  lcd.setCursor(0, 1);
  lcd.print("    NEGADO!");
  for (int i = 0; i < 11; i++) {
    tone(audioPin, 1080, 100);
    digitalWrite(yellowPin, LOW);
    digitalWrite(redPin, HIGH);
    delay(duration);
    noTone(audioPin);
    digitalWrite(redPin, LOW);
    delay(100);
  }
  delay(1000);
}



void get_new_pass() {
  new_pass_entry(); // mensagem, som e LED
  int entrada = 0; // inicializa entrada
  while (count < 4) { // enquanto contrador for menor que 4
    char key = keypad.getKey(); // obtem informação do teclado
    if (key != NO_KEY) { // se algo foi teclado
      entrada += 1; // aumenta contador de entrada
      tone(audioPin, 1080, 100); // tom para cada dígito
      delay(duration);
      noTone(audioPin);
      pass[count] = key; // armamazena o novo dígito
      count += 1; // próximo dígito
      if (count == 4) break; // chegou a 4 digitos, interrompe loop
      if ((key == '*') || (entrada == 4)) { // foi telcado * 4 entradas
        key_init();// inicializa sistema
        break; // mata o void
      }
    }
  }
}


void new_pass_entry() {
  // mensagem no display
  lcd.clear();
  lcd.print("   Nova Senha");
  count = 0;
  lcd.setCursor(0, 1);
  lcd.print("    Tecle *");
  // gera sinal audível
  tone(audioPin, 1500, 100);
  delay(duration);
  noTone(audioPin);
  tone(audioPin, 1500, 100);
  delay(duration);
  noTone(audioPin);
  tone(audioPin, 1500, 100);
  delay(duration);
  noTone(audioPin);
  // somente LED amarelo aceso
  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, HIGH);
  digitalWrite(greenPin, LOW);
}

void old_pass_check() {
  // mensagem no display
  lcd.clear();
  lcd.print("  Senha antiga?");
  count = 0;
  lcd.setCursor(0, 1);
  lcd.print("    Tecle *");
  // gera tom audível
  tone(audioPin, 1500, 100);
  delay(duration);
  noTone(audioPin);
  tone(audioPin, 1500, 100);
  delay(duration);
  noTone(audioPin);
  tone(audioPin, 1500, 100);
  delay(duration);
  noTone(audioPin);
  // somente LED amarelo aceso
  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, HIGH);
  digitalWrite(greenPin, LOW);
}


void key_init () {
  // mensagem no display
  lcd.clear();
  lcd.print("  Bem vindo...");
  lcd.setCursor(0, 1);
  lcd.print("  Tecle: # ou *");
  count = 0;// contador para zero
  // somente LED vermelho aceso
  digitalWrite(redPin, HIGH);
  digitalWrite(yellowPin, LOW);
  digitalWrite(greenPin, LOW);
  // gera tom audível
  tone(audioPin, 1080, 100);
  delay(duration);
  noTone(audioPin);
  tone(audioPin, 980, 100);
  delay(duration);
  noTone(audioPin);
  tone(audioPin, 770, 100);
  delay(duration);
  noTone(audioPin);
}

void code_entry_init() {
  // mensagem no display
  lcd.clear();
  lcd.print(" Entre a Senha:");
  count = 0; // contador para zero
  // gera sinal audível
  tone(audioPin, 1500, 100);
  delay(duration);
  noTone(audioPin);
  tone(audioPin, 1500, 100);
  delay(duration);
  noTone(audioPin);
  tone(audioPin, 1500, 100);
  delay(duration);
  noTone(audioPin);
  // somente LED amarelo aceso
  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, HIGH);
  digitalWrite(greenPin, LOW);
}

void unlocked() {
  // mensagem no display
  lcd.clear();
  lcd.print("Acesso Liberado!");
  // somente LED verde aceso
  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, LOW);
  // pisca LED verde e gera sinal audível
  for (int x = 0; x < 5; x++) {
    digitalWrite(greenPin, HIGH);
    tone(audioPin, 2000, 100);
    delay(duration);
    noTone(audioPin);
    digitalWrite(greenPin, LOW);
    tone(audioPin, 2000, 100);
    delay(duration);
    noTone(audioPin);
    delay(250);
  }
}

void locked_trace() {
  //mensagem no display 

  lcd.clear();
  lcd.print("Digite '#' para sair");
  // Detona LED Verde
  digitalWrite(redPin, LOW);
  digitalWrite(yellowPin, LOW);
  // piscar o LED verde e gerar sinal audivel
  loop();
}

