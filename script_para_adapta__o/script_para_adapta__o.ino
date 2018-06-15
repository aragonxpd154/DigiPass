#include <LiquidCrystal.h>
#include <LM35.h>
#include <limits.h>
#include <EEPROM.h>

// Pino analogico para ligacao do LM35
int pin = A0;

// Variaveis que armazenam a temperatura em Celsius e Fahrenheit
int tempc = 0, tempf = 0;

// Array para precisão na medição
int samples[8];

// Variáveis que guardam a temperatura máxima e mínima
int maxtemp = -100, mintemp = 100;

int menorValorTemp = INT_MAX;

int i;

// Variaveis da EEPROM determinantes para consulta

int endereco = 0; // Variavel de endereco de localizacao da memoria EEPROM
int dados = 0; // Variavel de dados a serem amarzenada na EEPROM
int VarTempEEPROM = 0; // Variavel usado para armazenar a leitura da EEPROM

long SetaLed = 7200000; // Valores de tempo de execucao da funcaoo exponencial.
long previsaoMillSegundos = 0; // Variavel de previsaoo milisegundos, que realizarar o inicio da contagem da funcao millis().
long intervaloBlack = 7200000; // Intervalo de comutacaoo descrita em (milisegundos) em funcao de millis().
int semana = 0; // Variavel tipo semana, determina o periodo em que vai comutar as chaves

// Variaveis controladora do tempo ligado dos equipamentos (CHAVE ??)

long acender2Min = 120000; // Tempo em ms de 15min de chave 1/chave 2 on
long intervaloInterrompido = 119999; // Intervalo de 15 min antes de entra novamente, para evitar alternanca de chave desnecessaria e aleatoria por variacao de tensao, caso ocorra uma harmonica no circuito ou qualquer coisa do genero
long previsaoMillSegundos15Min = 120000;
int temporizadorLogico = 0; // Variavel que determinara o atraso do contador de 30 min em outra funcao Millis() de 30 min para o delay das chaves comutadas

// Variaveis tipo temporaria
/* Essas variaveis sao para uso temporario, e seus valores iniciais nao sao reais, e soferam
    modificacoes ao longo do compilamento/depuramento do codigo fonte
*/

int valorSensorTemp = 0; // Variavel temporario do Sensor de temperatura #LM35
//int tempc = INT_MAX; // Variavel para enxergar o maior valor MAXIMO
int valorTensao = 0; // Capturar o valor da tensao em A1 e exibir via Serial/ou qualquer entrada de parametros externos.

/* Variaveis tipo pulso ou PWMPin sao variaveis estacionaria e direcionada para seus pino correspondente
    que realiza a comutacao e contagem independente se o pulso do DIGI MEC estiver indisponivel dentro da
    escala de tempo abaixo.*/


int pwmPin = 13; // Localizacao da chave (CHAVE 1)
int pwm2Pin = 12; // Localizacao da chave (CHAVE 2)

//Define os pinos que serão ligados ao LCD
LiquidCrystal lcd(10, 5, 4, 3, 11, 2);

//Array que desenha o simbolo de grau
byte a[8] = {B00110, B01001, B00110, B00000, B00000, B00000, B00000, B00000,};

void controle() {
  digitalWrite(pwmPin, HIGH);
  digitalWrite(pwm2Pin, HIGH);
  delay(9600);
  delay(9600);
  delay(9600);
  delay(9600);
  delay(9600);
  delay(9600);
  delay(9600);
  delay(9600);
  delay(9600);
  delay(9600);
  delay(9600);
  delay(9600);
  delay(9600);
  delay(9600);
  digitalWrite(pwmPin, LOW);
  digitalWrite(pwm2Pin, LOW);
}

void setup()
{
  Serial.begin(9600); // Inicializa comunicação serial
  pinMode(pwmPin, OUTPUT);
  pinMode(pwm2Pin, OUTPUT);
  lcd.begin(16, 2); //Inicializa
  lcd.print("INICIALIZANDO");


  Serial.print("LENDO MEMORIA EEPROM\n\n");
  VarTempEEPROM = (EEPROM.read(endereco));
  if (semana == VarTempEEPROM) {
    Serial.print("DADOS PREMATUROS\n\n");
  }
  else {
    semana = VarTempEEPROM;
  }

  if (semana == 0) {
    Serial.print("CHAVE 1 SEMPRE LIGADO ATE QUE COMUTE PWM NA VARIAVEL COMUTATIVA\n\n\n");
  }
  else {
    Serial.print("CHAVE 2 SEMPRE LIGADO ATE QUE COMUTE PWM NA VARIAVEL COMUTATIVA\n\n\n");
  }

  Serial.print("LIGANDO EQUIPAMENTOS\n");

  //controle();
  lcd.clear();
  lcd.print("Temperatura: ");
  //Atribui a "1" o valor do array "A", que desenha o simbolo de grau
  lcd.createChar(1, a);
  lcd.setCursor(7, 1); //Coloca o cursor na coluna 7, linha 1
  lcd.write(1); //Escreve o simbolo de grau
  lcd.setCursor(15, 0);
  lcd.write(1);
  lcd.setCursor(15, 1);
  lcd.write(1);
}


void loop()
{
  unsigned long tempoOcorrido = millis(); // Declarando a funcao millis() diante da variavel tempoOcorrido
  unsigned long tempoOcorrido15Min = millis(); // Declarando a funcao contadora millis() diante da variavel tempoOcorrido15Min

  // Loop que faz a leitura da temperatura 8 vezes
  for (i = 0; i <= 18; i++)
  {
    if (valorSensorTemp < menorValorTemp) {
      menorValorTemp = valorSensorTemp;

      Serial.print("\nTEMPERATURA DE:  ");
      Serial.print(menorValorTemp);
      Serial.print(" C");
      Serial.print("\n");
      Serial.print("\n");
      Serial.print("\n");
      Serial.print("TEMPERATURA BAIXA\n");
      Serial.print("COMUTACAO: ");
      Serial.println(semana);

      samples[i] = ( 5.0 * analogRead(pin) * 100.0) / 1023.0;
      //A cada leitura, incrementa o valor da variavel tempc
      tempc = tempc + samples[i];
      delay(100);
    }

    if (tempoOcorrido - previsaoMillSegundos > intervaloBlack) {
      // Salva o tempo anterior para referencial do tempo do LED sob o intervalo
      previsaoMillSegundos = tempoOcorrido;

      if (SetaLed == LOW) {
        SetaLed = HIGH;
        semana = 1;
        dados = 1; // Armazenando dado na variavel para enderecamento de memoria na EEPROM diante da condicao acima
        Serial.print("\nGRAVANDO DADOS ALTERADO DO PULSO SEMANAL NA MEMORIA EEPROM, AGUARDE");
        EEPROM.write(endereco, semana);
        delay(100);
        Serial.print("\n");
        Serial.print("ARMAZENADO COM SUCESSO");
      }
      else {
        SetaLed = LOW;
        semana = 0;
        dados = 0; // Armazenando dado na variavel para enderecamento de memoria na EEPROM diante da condicao acima
        Serial.print("\nGRAVANDO DADOS ALTERADO DO PULSO SEMANAL NA MEMORIA EEPROM, AGUARDE");
        EEPROM.write(endereco, semana);
        delay(100);
        Serial.print("\n");
        Serial.print("ARMAZENADO COM SUCESSO");
      }

      //digitalWrite(ledPin1, SetaLed);

      //Serial.print(semana); //**E TESTE

    }

    if (tempc <= 27) {
      if (semana == 1) {
        pwmPin = 13;
        pwm2Pin = 12;
      }
      else {
        pwmPin = 12;
        pwm2Pin = 13;
      }

      digitalWrite(pwmPin, LOW);
      digitalWrite(pwm2Pin, HIGH);


    }

    if (tempc >= 26) {

      /* Essa condicao definida para quando chegar certo C(graus) ligar todas as chaves para alto
         independente se estiverem comutadas.

      */
      if (semana == 1) {
        pwmPin = 13;
        pwm2Pin = 12;
      }
      else {
        pwmPin = 12;
        pwm2Pin = 13;
      }

      Serial.print("\n\nTEMPERATURA ALTA\nCHAVE 1 LIGADO\n");

      if (semana == 1, semana == 0) { // Independente que esteja a valor alto no pusante as chaves ainda estaao todas as 2 ligadas
        Serial.print("CHAVE 2 LIGADO\n");
      }
      else {
        //Serial.print("CHAVE 2 LIGADO\n");
      }
      // Serial.print(tempc);
      Serial.print("\n");

      if (tempoOcorrido15Min - previsaoMillSegundos15Min >= intervaloInterrompido) {
        previsaoMillSegundos15Min = tempoOcorrido15Min;
        temporizadorLogico = 1;
        acender2Min = LOW;

        if (acender2Min == LOW) {
          acender2Min = HIGH;
        }

        digitalWrite(pwmPin, acender2Min);
        digitalWrite(pwm2Pin, acender2Min);
      }
      //delay(150);
    }

    else {
      if (semana == 1) {
        pwmPin = 13;
        pwm2Pin = 12;
      }
      else {
        pwmPin = 12;
        pwm2Pin = 13;
      }

      // Serial.print(tempc); //* EM analise

      if (semana == 1) { // Comutando as chaves pelas condicoes do PWM ou variavel pulsante
        Serial.print("\nCHAVE 1 DESLIGADO E CHAVE 2 LIGADO");
      }
      else {
        Serial.print("\nCHAVE 1 LIGADO E CHAVE 2 DESLIGADO");
      }
      if (temporizadorLogico == 1) {
        if (tempoOcorrido15Min - previsaoMillSegundos15Min >= intervaloInterrompido) {
          previsaoMillSegundos15Min = tempoOcorrido15Min;
          temporizadorLogico = 0;
          acender2Min = LOW;

          if (acender2Min == LOW) {
            acender2Min = HIGH;
          }

          digitalWrite(pwmPin, acender2Min);
          digitalWrite(pwm2Pin, acender2Min);
        }
        digitalWrite(pwmPin, HIGH);
        digitalWrite(pwm2Pin, HIGH);
      }

      else  {
        if (tempc <= 27) {
          digitalWrite(pwmPin, LOW);
        }
        digitalWrite(pwm2Pin, HIGH);
      }
      //delay(9600); // repetir todo o ciclo de medicoes seguindo o banco de atraso anterior
      delayMicroseconds(10000);

    }


    if (tempc >= 100)
      /* Essa condicao vai determinar que acima de 100 C (graus), deduzindo que a leitura esteja
         divergente do real, e deduz que o componente LM35 em seu coletor, esteja saturado
         e levando em conta o atraso, por precaucoes as CHAVE'S 1 e 2, permanecerao ativas, ate que
         se resolva trocar o transistor LM35 ou averiguar, para a seguranca dos equipamentos que
         estarao ligado a esses reles DC.
      */
    {
      if (semana == 1) {
        pwmPin = 13;
        pwm2Pin = 12;
      }
      else {
        pwmPin = 12;
        pwm2Pin = 13;
      }


      // Divide a variavel tempc por 8, para obter precisão na medição
      tempc = tempc / 8.0;

      //Converte a temperatura em Fahrenheit e armazena na variável tempf
      tempf = (tempc * 9) / 5 + 32;

      //Armazena a temperatura máxima na variável maxtemp
      if (tempc > maxtemp) {
        maxtemp = tempc;
      }

      //Armazena a temperatura minima na variavel mintemp
      if (tempc < mintemp) {
        mintemp = tempc;
      }


      //As linhas abaixo escrevem o valor da temperatura na saída serial
      Serial.print(tempc, DEC);
      Serial.print(" Celsius, ");
      Serial.print(tempf, DEC);
      Serial.print(" fahrenheit -> ");
      Serial.print(maxtemp, DEC);
      Serial.print(" Max, ");
      Serial.print(mintemp, DEC);
      Serial.println(" Min");

      delay(100);
      lcd.setCursor(13, 0);
      lcd.print(tempc, DEC); //Escreve no display o valor da temperatura
      lcd.setCursor(0, 1);
      lcd.print("Min:");
      lcd.setCursor(5, 1); //Posiciona o cursor na coluna 5, linha 1 do display
      lcd.print(mintemp, DEC); //Escreve no display o valor da temperatura minima
      lcd.setCursor(9, 1); //Posiciona o cursor na coluna 9, linha 1 do display
      lcd.print("Max: ");  //Escreve no display o valor da temperatura maxima
      lcd.setCursor(13, 1);
      lcd.print(maxtemp, DEC);



      tempc = 0;
    }
  }
}

