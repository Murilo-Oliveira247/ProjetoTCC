#include <ESP8266WiFi.h>
#include <Ethernet.h>

//============================================================================
//                    DEFINIÇÕES DE PINOS
//============================================================================

//Definindo os pinos em que o sensor vai se comunicar com o Esp8266
#define pinTrigger 13
#define pinEcho 15

//Definindo o pino em que o Esp8266 faz a comunicação com o Rele
#define pin_rele D2

//============================================================================
//  Definindo variaveis que são utilizadas no loop para efetuar os calculos;
//============================================================================

float valor_aciona = 20.0;
float recebe_distancia;
float conversao1;
float conversao2;
float conversao3;

String envia_dados_grafico_1;
String envia_dados_grafico_2;
String envia_dados_grafico_3;

//===========================================================================
//          DEFININDO A COMUNICAÇÃO COM O THINGSPEAK
//===========================================================================

String apiWritekey = "VWW88ALGVCI0C9ET"; // substitua pela chave THINGSPEAK WRITEAPI aqui
const char* ssid = "TP-Link_2BBC"; // Nome da rede Wifi (SSID)
const char* password = "92275706" ;// Senha da Rede Wifi
 
const char* server = "api.thingspeak.com";

// 3,3 é a voltagem de alimentação e 1023 é o valor máximo de leitura analógica.
float resolution = (3.3/1023);


WiFiClient client;

void  setup ()  {
  
  // Iniciando a comunicação Serial;
  Serial.begin(115200);
  
  // Inicializando a saída do acionador e a entrada do eco
  pinMode (pinTrigger , OUTPUT);
  pinMode (pinEcho , INPUT_PULLUP);

  //Iniciando a conexão com o WiFi;
  WiFi.disconnect();
  delay(10);
  WiFi.begin(ssid, password);
 
 //Em caso de sucesso, mostrar em qual rede está conectado;
  Serial.println();
  Serial.print("Conectado a Rede: ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);

  // Enquanto aguarda a conexão;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("ESP8266 conectado ao WiFi: ");
  Serial.println(ssid);
  Serial.println();

  //Definindo o que o pino do Rele faz;
  pinMode(pin_rele, OUTPUT);
  digitalWrite(pin_rele, HIGH); // deixando o rele desligado em nível logico alto;

  } 
  
  void  loop ()  { 
    
    digitalWrite (pinTrigger, LOW);  // Defina o pino do gatilho para baixo para 2uS
    delayMicroseconds(2); 
    
    digitalWrite (pinTrigger , HIGH);  // Envie um 10uS alto para acionar o alcance
    delayMicroseconds(20); 
    
    digitalWrite (pinTrigger , LOW);  // Envie o pino para baixo novamente
    float  distancia  =  pulseIn (pinEcho , HIGH , 26000);  // Lê em tempos de pulso
    
    recebe_distancia =  distancia / 58 ;  // Converte a duração do pulso em distância, 58 é a velocidade da luz para o sensor ter um parametro quando recebe a onde de volta;
    conversao1 = recebe_distancia*1000; // transformando a leitura feita em cm para cm³ e armazendo na variavel conversao1;
    conversao2= (conversao1/1000); // pegando cm³ da variavel conversao1 e transformando para litros e armazenando o valor em conversao2;
    
  //Aqui utilizamos a função map para inverter a logica, ou seja, quando o sensor medir a distância total significa que o reservatorio está vazio;
    conversao3 = (map(conversao2, 0, 50, 50, 0)); 
                               
  //Mostrando o valor da distancia em Centimetro (Cm);
    Serial.println("Distância em cm: ");
    Serial.println(recebe_distancia);
    Serial.println("\n");

  //Mostrando a conversão para Centimetros cúbicos (cm³);
    Serial.println("Distância em cm³: ");
    Serial.println(conversao1);
    Serial.println("\n");

  //Mostrando a conversão para Litros (L);
    Serial.println("Litros: ");
    Serial.println(conversao3);
    Serial.println("\n");

    // Logica para fazer o acionamento do Rele;
   if (recebe_distancia >= valor_aciona){

     digitalWrite(pin_rele, LOW);
     Serial.println("Bomba Ligada nivel do reservatorio vazio");
     Serial.println("\n");

   }
   else{
     digitalWrite(pin_rele, HIGH);
     Serial.println("Bomba Desligada, nivel do reservatorio cheio");
     Serial.println("\n");
  
    }

  //Tranformando os  dados das variaveis floats para String para serem enviadas para o gráfico;
    envia_dados_grafico_1 = recebe_distancia;
    envia_dados_grafico_2 = conversao1;
    envia_dados_grafico_3 = conversao3;


    float temp = (analogRead(A0) * resolution) * 10;
    
    if (client.connect(server, 80))
    { 
          String tsData = apiWritekey;
                tsData +="&field1=";
                tsData += String(recebe_distancia);
                tsData += "\r\n\r\n";
                tsData +="&field2=";
                tsData += String(conversao1);
                tsData += "\r\n\r\n";     
                tsData +="&field3=";
                tsData += String(conversao3);
                tsData += "\r\n\r\n";
      
          client.print("POST /update HTTP/1.1\n");
          client.print("Host: api.thingspeak.com\n");
          client.print("Connection: close\n");
          client.print("X-THINGSPEAKAPIKEY: "+apiWritekey+"\n");
          client.print("Content-Type: application/x-www-form-urlencoded\n");
          client.print("Content-Length: ");
          client.print(tsData.length());
          client.print("\n\n");  // os 2 retornos indicam o fechamento dos campos do cabeçalho e o início dos dados;
          client.print(tsData);
      
          Serial.print("distancia em cm: ");
          Serial.print(recebe_distancia);
          Serial.print("distancia em cm³: ");
          Serial.print(conversao1);
          Serial.print("Volume em L: ");
          Serial.print(conversao3);     

          Serial.println("carregado para o servidor Thingspeak....");
    }
      client.stop();
    
    // Serial.println("Agardando para fazer o upload da próxima leitura...");
      Serial.println();
      
      // Thingspeak nescessita de um atraso mínimo de 15 segundos entre as atualizações;
      delay(15000);

  }