/* PROGRAMMA FV AUTOCONSUMO ZERO CON 2 SENSORI DIGITALI, "PENELOPE" E CON PIU' USCITE PWM, VERSIONE "ARIANNA"
 Versione per display 2x16 SERIALE (A4 e A5)
 Allaccio contatori a A1 e A2
 Uscite PWM: D6 e D9 per "rubinetto" Arianna:
 Uscita PWM: D10 per RSS boiler: 
 autore SoftPlus Consumo Zero - email: luigi.marchi.faenza@gmail.com  
 Aggiornamento software del 17 settembre 2016:
 //
 //
 ATTENZIONE, IMPORTANTE: 
 Arduino funziona a 5 volt e non ha problemi di sorta, circa il rischio di scariche elettriche.                                 
 Circa l'alimentatore da rete è OBBLIGATORIO utilizzare un caricabatteria da telefonino (a norma) con uscita 8, 9 volt DC.    
 Per operare sulle parti hardware di contorno occorre avere le competenze ed esperienze consolidate. 
 Occorre essere ESPERTI e CON PATENTINO per gli interventi SUL QUADRO ELETTRICO e in apparecchiature con tensioni di rete (230 AC)
 e/o tensioni elevate di stringhe di pannelli fotovoltaici. E' pertanto necessario affidarsi a personale qualificato.                                 
 Il produttore del software declina ogni responsabilità per danni elettrici.                                                  
 Quanto ai danni derivante da malfunzionamento del software, il produttore del software declina ogni responsabilità circa:    
 1) danni derivanti da utilizzo di componentistica di contorno;                                                               
 2) danni derivanti da modifica del software originario;                                                                      
 3) danni derivanti da scarsa esperienza e scarsa capacità professionale dell'installatore.                                                
 //
 //
 IMPORTANTE: i due sensori da quadro vanno collocati: uno sulla linea dei consumi domestici, l'altro sulla linea che va all'inverter
 il relè a stato solido per il carico (boiler, stufette, scaldabagni) va inserito sulla linea dei consumi, perchè, per equilibrare
 produzione e consumi, si deve misurare, fra i consumi, anche quelli del carico aggiuntivo. 
 //
 IMPORTANTE: i sensori digitali producono degli impulsi la cui distanza, l'uno dall'altro, è inversamente proporzionale alla potenza:
 ciò comporta una certa lentezza nella valutazione dei piccoli carichi,la contropartita è una misurazione "fiscale" della massima
 precisione, come i contatori Enel; i sensori utilizzati (reperibili presso ELCOTRONIC Srl) richiedono 3200 impulsi ora per un KWora.
 //
 Basetta di interfaccia, tra contatori e Arduino: 2 resistenze da 39 Kohm lato massa e 2 da 100 ohm lato 5 volt.
 Relativamente al numero di impulsi, va utilizzato il software relativo, oppure modificare le istruzioni che fanno riferimento 
 agli impulsi (32 per 3200, 20 per 2000, 10 per 1000, ecc.) 
 Non ci sono trimmer di taratura, neppure software:
 //
 */
#include <Arduino.h> //inclusione della libreria base:
// includere altre librerie:
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
// LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);    //POSITIVE=accensione retroilluminazione:
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);    // indirizzo LCD Philips PCF8574AT: 
//
// definizione delle variabili e dei pin di INPUT e OUTPUT:
int pin_carico1 = 6;        // carico analogico PWM1:
int pin_carico2 = 9;        // carico analogico PWM2:
int pin_carico3 = 10;        // carico analogico PWM3 per boiler:
//

int pin_consumi = A2;       // pin dove arrivano gli impulsi dei consumi:
int pin_produzione = A1;    // pin dove arrivano gli impulsi della produzione fotovoltaica:
int pin_GRID = A0;       // pin dove arrivano gli impulsi dei consumi:
int long ciclopwm_RSS = 0;
int long ciclopwm = 0;       // va bene:
// int ciclopwm = 0;                    // questo va bene:
int ciclopwm1 = 0;                    // questo va bene:
int ciclopwm2 = 0;                    // questo va bene:
int ciclopwm3 = 0;                    // questo va bene:
unsigned long conta1 = 0;
unsigned long conta2 = 0;
unsigned long conta3 = 0;
unsigned long val_prod = 0;
unsigned long val_cons = 0;
int aumento = 0;
int potenza_watt_consumi = 0;
int potenza_watt_produzione = 0;
int buttonState = 0;
int modificaOK = 0;
int aumentoOK = 0;
unsigned long vecchio_millis_consumi = 0;
unsigned long corrente_millis_consumi = 0;
unsigned long tempo_trascorso_consumi = 0;
unsigned long vecchio_millis_produzione = 0;
unsigned long corrente_millis_produzione = 0;
unsigned long tempo_trascorso_produzione = 0;
unsigned long tempo_trascorso = 0;
unsigned long tempo_vecchio = 0;
unsigned long tempo_nuovo = 0;
unsigned long stop_tempo = 0;
unsigned long val;
unsigned long val1;
unsigned long val2;
unsigned int cicli = 0 ;
unsigned long sommaFV = 0;
unsigned long sommaCons = 0;
unsigned long somma_GRID = 0;
unsigned long millesimi = 0;
unsigned long mill_produzione = 0;
unsigned long mill_consumi = 0;
unsigned long tempo_adesso_consumi = 0;
unsigned long tempo_adesso_produzione = 0;
int volt_partitore_consumi = 0;
int volt_partitore_produzione = 0;
int modifica_rilevazioni = 0;
int volt_GRID = 0;
// ***************************************************************
int impulsi_kw = 3200;                  // dato modificabile:
// ***************************************************************
// altre definizioni:
void setup()
{
  pinMode(pin_carico1, OUTPUT);   //pin digitale 6 uscita carico PWM:  
  pinMode(pin_carico2, OUTPUT);   //pin digitale 9 uscita carico PWM: 
  pinMode(A0, INPUT);             //pin sensore analogico tensione GRID:  
  pinMode(A1, INPUT);             //pin sensore digitale produzione:
  pinMode(A2, INPUT);             //pin sensore digitale consumi:
  lcd.begin(16,2); // Inizializzo LCD 16 caratteri per 2 linee, SERIALE:
}
//
// Inizia ciclo
void loop() {
  //
  //
  // step 1 lettura dei dati digitali:
  //
  for (int cicli = 0; cicli < 10; cicli++)
  {
    volt_GRID = analogRead(A0);
    somma_GRID = (somma_GRID + volt_GRID);
    delay(2);
  }  
  volt_GRID = somma_GRID / 10;
  volt_GRID = volt_GRID + 50;       // correzione per caduta tensione diodo raddrizzatore:   
  //  volt_GRID = (volt_GRID / 2.98);      // parametro di regolazione fine della tensione (tipo 1): 
  volt_GRID = (volt_GRID / 2.94);      // parametro di regolazione fine della tensione (tipo 2): 
  // volt_GRID = (volt_GRID / 3.42);      // parametro di regolazione fine della tensione (tipo 3):     
  somma_GRID = 0;
  //
  //
  //
  //
  tempo_adesso_produzione = millis();
  if (tempo_adesso_produzione > vecchio_millis_produzione + 100)   // bisogna che siano passati almeno 100 millisecondi dall'ultima lettura:
  {
    // legge lo stato e percepisce l'impulso sul pin deputato a rilevare la produzione:
    volt_partitore_produzione = analogRead(A1);
    if (volt_partitore_produzione > 200)       // la tensione deve essere maggiore di 1 volt:    
    {
      corrente_millis_produzione = millis();
      tempo_trascorso_produzione = corrente_millis_produzione - vecchio_millis_produzione;
      vecchio_millis_produzione = corrente_millis_produzione;
      // calcolo produzione media, in watt, di tempo in tempo:
      // 3200 impulsi ora per 1 Kwh, cioè 1.000 wattora:
      // cioè per ogni 1000 wattora serve 1 impulso ogni 3600/3200 secondi cioè ogni 1,125 secondi:
      // cioè se gli impulsi arrivano ogni 11,25 secondi abbiamo 100 wattora e così via:
      // poi bisogna tradurre in millesimi, laddove 1,125 secondi sono 1.125 millesimi di secondo:
      // potenza_watt_produzione = 1000000 / tempo_trascorso_produzione * 36 / 32;       //valido per contatore da 3200 impulsi:
      // potenza_watt_produzione = 1000000 / tempo_trascorso_produzione * 36 / 20;       //valido per contatore da 2000 impulsi:      
      // potenza_watt_produzione = 1000000 / tempo_trascorso_produzione * 36 / 16;       //valido per contatore da 1600 impulsi:
      // potenza_watt_produzione = 1000000 / tempo_trascorso_produzione * 36 / 10;       //valido per contatore da 1000 impulsi:  
      potenza_watt_produzione = 1000000 / tempo_trascorso_produzione * 3600 / impulsi_kw;   
      val_prod = potenza_watt_produzione;
      val1 = val_prod;
      modifica_rilevazioni = 1;
      //    buttonState = 0;
      //    aumentoOK = 0;
      //    delay(100);        // ritardo per stoppare la ricerca degli impulsi e evitare disguidi:
    }
  }
  //
  //
  conta1 = (conta1 + 1);
  conta2 = (conta2 + 1);
  conta3 = (conta3 + 1);
  //
  //
  // step 2 modifica del cicloPWM per azzerare il differenziale tra produzione e consumi:
  //
  // faccio la modifica ogni volta che arriva un nuovo impulso dal contatore dei consumi o da quello dela produzione:
  // step 2 modifica del cicloPWM per azzerare il differenziale tra produzione e consumi:
  //
  //
  tempo_nuovo = millis();
  if (modifica_rilevazioni == 1 || tempo_nuovo - tempo_vecchio > 3000)  // fa l'aggiornamento ogni 3 secondi o più in fretta:
  {
    tempo_vecchio = tempo_nuovo;
    if (millis() > tempo_trascorso + 500)     // vogliamo che passi un pò di tempo, per il ricalcolo del differenziale:
      // if (millis() > tempo_trascorso + 200)     // vogliamo che passi un pò di tempo, per il ricalcolo:    
    {
      modifica_rilevazioni = 0;
      // facciamo una verifica dello sbilancio e modifichiamo i valori di PWM:
      aumento = 3 + (ciclopwm / 20);   
      //      if (volt_GRID > 252) ciclopwm = ciclopwm - aumento;    
      if (volt_GRID > 252) ciclopwm = ciclopwm - aumento;          
      else ciclopwm = ciclopwm + aumento;  
      // imposta i valori limite del ciclo_PWM (da 0 a 255):
      if (ciclopwm < 1) ciclopwm = 0;
      if (ciclopwm > 255) ciclopwm = 255;
      analogWrite(pin_carico1, ciclopwm);  
      analogWrite(pin_carico2, ciclopwm); 
      //
      aumentoOK = 1;       //mettiamo a "1" la variazione di aumento per evitare di ripeterla prima delle nuove misure:
      modifica_rilevazioni = 0;
      tempo_trascorso = millis();
    }
  }
  //
  //
  // step 3 gestione del display relativamente a consumi ( in watt) e produzione fotovoltaico (in watt):
  // la visualizzazione dei dati avviene solo dopo un certo numero di cicli di verifiche Arduino:
  //
  if (conta1 > 100)     // lo faccio funzionare solo ogni 2 secondi circa: 
  {
    conta2 = 0;
    conta1 = 0;
    lcd.clear();      // pulisce lo schermo:
    lcd.setCursor(0, 0);
    lcd.print("Volt_RETE  W_FV ");
    lcd.setCursor(1, 1);
    lcd.print(volt_GRID);
    lcd.setCursor(11, 1);
    lcd.print(val1);
  }
  //

  // step 5 gestione del display relativamente a prelievi dalla rete (Enel) o immissioni in rete (GSE), l'uno alternativo all'altro:
  // inoltre viene evidenziato il carico extra per pareggiare produzione e consumi(i cicli PWM vanno da 1 a 254),:
  // ma sono espressi in percentuale (da 0% a 100%):
  //
  if (conta2 > 50)            // deve visualizzare, a rotazione, solo ogni 2 secondi circa:  
  {
    conta2 = 0;
    lcd.clear();      // pulisce lo schermo:
    // Visualizzo il messaggio sul display SERIALE con le informazioni:    
    lcd.print("Volt_RETE  %PWM");
    lcd.setCursor(1, 1);
    lcd.print(volt_GRID);
    lcd.setCursor(11, 1);
    //    if (ciclopwm > 1)
    lcd.print(ciclopwm * 100 / 254);
    //    else
    //      lcd.print("O");
  }
}
// FINE listato by SoftPlus Consumo Zero - Faenza. 





















