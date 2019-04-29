//Agradecimentos:
//Augusto, Claudinei, Bontempo, Hefesto, Atena e Hermes/Mercúrio
//Biblioteca para uso do Wifi-Manager
#include <ESP8266WiFi.h>   
//#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

//Biblioteca MySQL
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>

//Biblioteca para uso do Servo
#include <Servo.h>

//Biblioteca do RTC (Real Time Clock)
#include <virtuabotixRTC.h>

//Biblioteca para uso da EEPROM
#include <EEPROM.h>

//Biblioteca para uso do SPIFFS
#include <FS.h>

//Define a alocação de memória pra EEPROM
#define MEM_ALOC_SIZE 16
//Defire o tamanho do array de string dos horarios
#define H_SPIFFS_SIZE 9
//Setando o número Serial da Máquina
  char SERIE_NUMBER[] = "613574";

//Controle de cadastro
  uint8_t cadastrada = 0;
  uint8_t horarios = 0;

//Configurando os dados do MySQL
  IPAddress server_addr(45,79,76,148);      // IP of the MySQL server
  char user[] = "apolo_esp";                // MySQL user login username
  char password[] = "apoloanhembi2017";     // MySQL user login password
  char database[] = "USE olympus";        //Database selection

//Variavel nome e senha de rede
  String line_SSID = "x";
  String line_PSK = "y";

//Pra saber o status da rede
  int wifiStatus;

//Query para o Insert no Banco
  char query_insert[] = "INSERT INTO `apolo_pill`(`serial`) VALUE (613574)";

//Query para o Select no Banco
  char query_select[] = "Select `H1`,`H2`,`H3`,`H4`,`H5`,`H6`,`H7`,`H8`,`H9` FROM `apolo_pill` WHERE `serial` = 613574";

//Query para o Update quanto ele toma
  char query_update[] = "UPDATE apolo_pill SET TOMOU = TOMOU + 1 WHERE SERIAL = 613574";

//Query para o Update quanto ele não toma
  char query_n_update[] = "UPDATE apolo_pill SET N_TOMOU = N_TOMOU + 1 WHERE SERIAL = 613574";
  
//Controle se está conectado a internet
  int conectado = 0;

//Necessário para o WiFi-Manager ou MySQL
  WiFiClient client;

//Necessário para o MySQL
 MySQL_Connection conn((Client *)&client);

//Cria o objeto RTC
virtuabotixRTC myRTC(13, 12, 14);

//váriavel de controle pra ler SSID e SENHA se não conectado
 int c_linha = 0;
  
//Variaveis com os horarios
  String hora1;
  String hora2;
  String hora3;
  String hora4;
  String hora5;
  String hora6;
  String hora7;
  String hora8;
  String hora9;

  int hora_atual;
  int minuto_atual;

  String horarios_spiffs[H_SPIFFS_SIZE] = { "9999", "9999", "9999", "9999", "9999", "9999", "9999", "9999", "9999" };

//Váriavel de controle da requisição dos horários
  int rede_download;

//Váriavel de controle do alarme e remédio
 int c_alarme = 0;
 int c_tomou = 0;
 int c_n_tomou = 0;

//Criação dos objetos servos
Servo black_servo;
Servo blue_servo;

//Definindo o botão e o copo
#define copo 2
#define btn_wifi 0

//Void quando toma
void tomou(){
MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
  
  //Seleciona a Database
  cur_mem->execute(database);

  //Executa o Select
  cur_mem->execute(query_update);
  
delete cur_mem;  
}

//Void quando não toma
void n_tomou(){
MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
  
  //Seleciona a Database
  cur_mem->execute(database);

  //Executa o Select
  cur_mem->execute(query_n_update);

 delete cur_mem;  
}


//Void Alarme
void alarme(){
  int inicio = millis();
    const int tempo_maximo = 4 * 60 * 1000;
     while (digitalRead(2) == HIGH && millis() < tempo_maximo + inicio) {
      digitalWrite(15, HIGH);
      delay(300);
      digitalWrite(15, LOW);
      delay(300);
      digitalWrite(15, HIGH);
      delay(300);
      digitalWrite(15, LOW);
      delay(1000);
   }
   if(digitalRead(2) == LOW){
    c_alarme++;
   }
}
  
//Void que dropa a capsula do A
void drop_a(){
    Serial.println("Dentro do Void Drop_A");
    if(digitalRead(2) == HIGH){
      alarme();
    }else{
      c_alarme++;
    }
    if(c_alarme == 0){
      c_n_tomou++;
    }else{
      //MEXE O SERVO A
      black_servo.write(0);
      delay(600);
      black_servo.write(37);
      delay(600);
      blue_servo.write(120);
      delay(800);
      blue_servo.write(0);
      delay(800);
      black_servo.write(0);
      c_tomou++;
      c_alarme = 0;
    }
    
}

//Void que dropa a capsula do B
void drop_b(){
  Serial.println("Dentro do Void Drop_B");
  if(digitalRead(2) == HIGH){
      alarme();
    }else{
      c_alarme++;
    }
    if(c_alarme == 0){
      c_n_tomou++;
    }else{
      //MEXE O SERVO B
      black_servo.write(0);
      delay(600);
      black_servo.write(77);
      delay(600);
      blue_servo.write(120);
      delay(800);
      blue_servo.write(0);
      delay(800);
      black_servo.write(0);
      c_tomou++;
      c_alarme = 0;
    }
    
 }

//Void que dropa a capsula do C
void drop_c(){
  Serial.println("Dentro do Void Drop_C");
  if(digitalRead(2) == HIGH){
      alarme();
    }else{
      c_alarme++;
    }
    if(c_alarme == 0){
      c_n_tomou++;
    }else{
      //MEXE O SERVO C
      black_servo.write(0);
      delay(600);
      black_servo.write(115);
      delay(600);
      blue_servo.write(120);
      delay(800);
      blue_servo.write(0);
      delay(800);
      black_servo.write(0);
      delay(500);
      
      c_tomou++;
      c_alarme = 0;
    }

}

//Void que compara e chama os voids A, B ou C
void comparar(){
  
  //Traz de volta os horários salvos no SPIFFS

  Serial.println("Iniciando o void comparar...");
  int i = 0;
  SPIFFS.begin();
  File hFile = SPIFFS.open("/horario_data.txt","r");

  Serial.println("Mostrando os horários salvos no horario_data.txt");
  for(int i=0; i <=8; i++){
    horarios_spiffs[i] = hFile.readStringUntil('\n');
    Serial.println(horarios_spiffs[i]);
  };
  
  //Strings que vão ser utilizadas
  String minuto_atual;
  String hora_atual;
  String tempo_atual;

  //Código que arruma as váriaveis dos horários;
  if(myRTC.minutes < 10){
    minuto_atual = "0" + String(myRTC.minutes);
  }else{
    minuto_atual = String(myRTC.minutes);
  };

  if(myRTC.hours < 10){
    hora_atual = "0" + String(myRTC.hours);
  }else{
    hora_atual = String(myRTC.hours);
  };
  tempo_atual = hora_atual + minuto_atual;

  for(int i = 0; i <= 8; i++){
    horarios_spiffs[i] = horarios_spiffs[i].substring(0,(horarios_spiffs[i].length() - 1));
  };

  // Verifica qual void deve rodar
  if(horarios_spiffs[0] == tempo_atual){
    drop_a();
  };
  if(horarios_spiffs[1] == tempo_atual){
    drop_a();
  };
  if(horarios_spiffs[2] == tempo_atual){
    drop_a();
  };
  if(horarios_spiffs[3] == tempo_atual){
    drop_b();
  };
  if(horarios_spiffs[4] == tempo_atual){
    drop_b();
  };
  if(horarios_spiffs[5] == tempo_atual){
    drop_b();
  };
  if(horarios_spiffs[6] == tempo_atual){
    drop_c();
  };
  if(horarios_spiffs[7] == tempo_atual){
    drop_c();
  };
  if(horarios_spiffs[8] == tempo_atual){
    drop_c();
  };
  if(c_tomou != 0){
    tomou();
    c_tomou = 0;
  }
  if(c_n_tomou != 0){
    n_tomou();
    c_n_tomou = 0;
  }
}

//Void que faz o select dos horários e salva
void horarios_download(){
  Serial.println("Void Horario_download iniciado");
  
  Serial.println("DB - Connecting...");
  while (conn.connect(server_addr, 3306, user, password) != true) {
    delay(500);
    Serial.print ( "." );
  }

  //Seleciona o Banco de Dados
  MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
  
  //Seleciona a Database
  cur_mem->execute(database);

  //Executa o Select
  cur_mem->execute(query_select);

  //Dá um fetch nas colunas (necessário)
  column_names *cols = cur_mem->get_columns();

  //Lê os valores e salva nas váriaveis que vão ser colocadas no SPIFFS
  row_values *row = NULL;
  do{
    row = cur_mem->get_next_row();
  if (row != NULL) {
        hora1 = row->values[0];
        hora1.trim();
 
        hora2 = row->values[1];
        hora2.trim();

        hora3 = row->values[2];
        hora3.trim();

        hora4 = row->values[3];
        hora4.trim();

        hora5 = row->values[4];
        hora5.trim();

        hora6 = row->values[5];
        hora6.trim();

        hora7 = row->values[6];
        hora7.trim();

        hora8 = row->values[7];
        hora8.trim();

        hora9 = row->values[8];
        hora9.trim();

        Serial.println(hora1);
        Serial.println(hora2);
        Serial.println(hora3);
        Serial.println(hora4);
        Serial.println(hora5);
        Serial.println(hora6);
        Serial.println(hora7);
        Serial.println(hora8);
        Serial.println(hora9);

        SPIFFS.begin();
          //re-cria o Spiffs se ele já existir
          if(SPIFFS.exists("/horario_data.txt")){
            Serial.println("Já este um arquivo com esse nome, recriando ...");
            SPIFFS.remove("/horario_data.txt");
            File wFile;
            wFile = SPIFFS.open("/horario_data.txt","w+");
          };
        File h = SPIFFS.open("/horario_data.txt", "a+");
        if(!h){
          Serial.println("Falha ao abrir horarios_data"); 
        }else{
          Serial.println("horario_data aberto com sucesso");
          h.println(hora1);
          h.println(hora2);
          h.println(hora3);
          h.println(hora4);
          h.println(hora5);
          h.println(hora6);
          h.println(hora7);
          h.println(hora8);
          h.println(hora9);
          Serial.println("Os horários foram escritos com sucesso no arquivo horarios_data");
        };
        h.close();
        
        //Salva na localização 3 da EEPROM um valor 1 como controle para ser utilizado depois
        Serial.println("Salvando a variavel de controle dos horarios");
        EEPROM.begin(MEM_ALOC_SIZE);
        EEPROM.write(3,1);
        EEPROM.end();

        Serial.println("Saindo do void horarios_download");
        delay(60000);
        
        
    }
  }while (row != NULL);
  delete cur_mem;  
}

//Void conexão com a internet
  void conecta(){
    Serial.println("Rodando o void Conecta");
    SPIFFS.begin();
    File rFile = SPIFFS.open("/wifi_data.txt","r");
    Serial.println("Lendo wifi_data");
    while(rFile.available()) {
      if (c_linha == 0){
      line_SSID = rFile.readStringUntil('\n');
      Serial.println(line_SSID);
      c_linha++;
      }else{
      line_PSK = rFile.readStringUntil('\n');
      Serial.println(line_PSK);
      };
    };
    rFile.close();
    SPIFFS.end();
    Serial.println(line_SSID.length());
    Serial.print(line_SSID);
    Serial.print("aqui");

    //Remove o NULL do final da String
    String ssidlast = line_SSID.substring(0,(line_SSID.length() - 1));
    String passlast = line_PSK.substring(0,(line_PSK.length() - 1));

    Serial.println();
    Serial.println();
    Serial.print("Conectando a ");
    Serial.println(ssidlast);
    
  //Tenta se conectar a rede cadastrada no SPIFFS
    Serial.println("Tentando se conectar com a rede cadastrada");
    WiFi.begin(ssidlast.c_str(), passlast.c_str());

    int inicio = millis();
    const int tempo_maximo = 1 * 60 * 1000;
     while (WiFi.status() != WL_CONNECTED && millis() < tempo_maximo + inicio) {
        delay(500);
        Serial.print(".");
      };
  }

//Void que é rodado uma vez
void setup(){
  //Começa a comunicação com o serial
  Serial.begin(115200);

  //Define o pino que o servo está
  Serial.println("Fazendo Attach dos servos");
black_servo.attach(5); //D1
blue_servo.attach(4); //D2

black_servo.write(0);

blue_servo.write(0);

delay(600);
pinMode(copo, INPUT_PULLUP);
pinMode(btn_wifi, INPUT_PULLUP);

pinMode(15, OUTPUT);
/* //Para resetar o WifiManager
WiFiManager wifiManager;
wifiManager.resetSettings();*/

  //Inicia a EEPROM alocando 16bytes
  EEPROM.begin(MEM_ALOC_SIZE);
  //EEPROM.write(0,0);
  //EEPROM.write(1,0);
  cadastrada = EEPROM.read(0);
  Serial.println("AGORA VAI MOSTRAR SE TÁ CADASTRADA SE FOR 0 TÁ OK");
  Serial.print(cadastrada);
  horarios = EEPROM.read(1);
  Serial.println(horarios);
  EEPROM.end();
  
  //Roda se a Máquina não for cadastrada
  if(cadastrada == 0){
    Serial.print("Máquina não cadastrada, iniciando o wifi-Manager");
    Serial.println();
    //Cria uma rede com nome ApoloPill
    WiFiManager wifiManager;
    wifiManager.resetSettings();
    wifiManager.autoConnect("ApoloPill");
    Serial.println("Conectado a Rede Com Sucesso");


    //Cria o arquivo wifi_data no SPIFFS, se ele já existir é deletado e então recriado.
    SPIFFS.begin();
    File wFile;
    if(SPIFFS.exists("/wifi_data.txt")){
      Serial.println("Arquivo wifi_data já existente! Recriando");
      SPIFFS.remove("/wifi_data.txt");  
      wFile = SPIFFS.open("/wifi_data.txt","w+");
      if(!wFile){
        Serial.println("Erro ao criar wifi_data");
      }else{
        Serial.println("Arquivo wifi_data criado com sucesso");
      };
      wFile.close();
    }else{
      wFile = SPIFFS.open("/wifi_data.txt","w+");
      if(!wFile){
        Serial.println("Erro ao criar wifi_data");
      }else{
        Serial.println("Arquivo wifi_data criado com sucesso");
      };
      wFile.close();
    };

    //Cria o arquivo horario_data.txt
    File yFile;
    if(SPIFFS.exists("/horario_data.txt")){
      Serial.println("Arquivo horario_data já existente! Recriando");
      SPIFFS.remove("/horario_data.txt");  
      yFile = SPIFFS.open("/horario_data.txt","w+");
      if(!yFile){
        Serial.println("Erro ao recriar horario_data");
      }else{
        Serial.println("Arquivo horario_data recriado com sucesso");
      };
      yFile.close();
    }else{
      yFile = SPIFFS.open("/horario_data.txt","w+");
      if(!yFile){
        Serial.println("Erro ao criar horario_data");
      }else{
        Serial.println("Arquivo horario_data criado com sucesso");
      };
      yFile.close();
    };

    //Salva o SSID e a Senha da rede no SPIFFS
    File f = SPIFFS.open("/wifi_data.txt", "a+");
    if (!f) {
    Serial.println("Falha ao abrir wifi_data.txt");
     }else{
       Serial.println("wifi_data aberto com sucesso");
       f.println(WiFi.SSID());
       f.println(WiFi.psk());
       Serial.println("SSID E PSK escrito no wifi_data");
     };
    f.close();

    //Conecta com o Banco de Dados
    Serial.println("DB - Connecting...");
    while (conn.connect(server_addr, 3306, user, password) != true) {
      delay(500);
      Serial.print ( "." );
    }
    SPIFFS.end();
    
    //Escolhe a database
    MySQL_Cursor cur = MySQL_Cursor(&conn);
    MySQL_Cursor *cur_mem = new MySQL_Cursor(&conn);
   
    cur_mem->execute(database);
    delete cur_mem;
    
    //Insere o Serial no Banco de Dados
    Serial.println("Fazendo o Insert do Serial no Banco");
    cur.execute(query_insert);
    cur.close();
    Serial.println("Insert Realizado com Sucesso");
    
    //Seta a máquina como Cadastrada
    EEPROM.begin(MEM_ALOC_SIZE);
    EEPROM.write(0,1);
    EEPROM.end();
    Serial.println("Máquina Cadastrada");

    //Seta que a máquina já está conectada a internet
    conectado = 1;
  }else{
    Serial.println("Máquina já cadastrada");
    Serial.println("Tentando se conectar a internet");
    conecta();
  };
}

//Void que roda em loop

void loop(){
  Serial.println("Atualizando o tempo do RTC");
  myRTC.updateTime(); //atualiza os horários do RTC
  String minutes_rtc; // Criar uma váriavel que pega os minutos do RTC aqui
  String final_three = "3";
  String final_five = "5";
  String final_zero = "0";

  //Se o rtc trouxer um número menor que 9 é adicionado um zero
  if(myRTC.minutes < 10){
    minutes_rtc = "0" + String(myRTC.minutes);
  }else{
    minutes_rtc = String(myRTC.minutes);
  };
  Serial.println(minutes_rtc);

  //Vê se o horário acaba com 3, se sim seta que o download deve ser feito
  if(minutes_rtc.endsWith(final_three)){
    rede_download = 1;
  }else{
    rede_download = 0;  
  };

  //Verifica se está conectado a Internet.
   wifiStatus = WiFi.status();
      if(wifiStatus == WL_CONNECTED){
         conectado = 1;
        }  
      else{
        conectado = 0;
      }

      //Ver esse delay depois
      delay(5000);
  
  if(conectado == 1){
    Serial.println("Conectado a Internet!!");
    
    //Se o horário for igual a 3 no final o if abaixo é rodado
    //Setar um tempo máximo pra isso depois
    if(rede_download == 1){
      horarios_download();
      delay(60000);
    };
  }else{
    Serial.println("Não conectado a Internet");
    //Chamo o Void que faz a conexão;
    void conecta();
  };


  //Verifica se o minuto acaba com 5
  if(minutes_rtc.endsWith(final_five) || minutes_rtc.endsWith(final_zero)){
    uint8_t tem_horario;
    
    EEPROM.begin(MEM_ALOC_SIZE);
    tem_horario = EEPROM.read(3);
    EEPROM.end();
    if(tem_horario == 1){
      Serial.println("Indo para o void de comparação");
      if(btn_wifi == 0){
        comparar();
      }else{
        alarme();
      }
      delay(60000);
    }else{
      Serial.println("Os horários não estão salvo no SPIFFS");
    };
  }else{
    Serial.println("Não é final 0,3 ou 5");
    //Testando os servos e o copo
    Serial.println("Se 0 o copo está encaixado");
    Serial.println(digitalRead(2));
    delay(600);
  };
  if(digitalRead(btn_wifi) == LOW){
    WiFiManager wifiManager;
    wifiManager.resetSettings();
  }
 
}
