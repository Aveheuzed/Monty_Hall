#include <Adafruit_NeoPixel.h>
#include <Servo.h>

#define PORT_BOUTON 2
#define PORT_SWITCH 3
#define PORT_BARRE_LED_G 4
#define PORT_BARRE_LED_D 5
#define PORT_BUZZER 6

#define NB_PORTES 5
#define LONGUEUR_BARRE_LED 10
#define NB_SEGMENTS 7

void manche(const unsigned int n, const unsigned int k, bool *change, bool *gagne);
void afficher(char chiffre);
void choisir_porte(unsigned char etat[], unsigned long int *i, char valeur);
void gerer_victoire(bool *change, bool *gagne);


const unsigned char PORT_LED_PORTE[NB_PORTES]={7,8,9,10,11};
const unsigned char PORT_BOUTON_PORTE[NB_PORTES]={14,15,16,17,18};
const unsigned char PORT_SERVO_PORTE[NB_PORTES]={19,20,21,22,23};
const unsigned char PORT_SERVO_VOITURE[NB_PORTES]={24,25,26,27,28};
const unsigned char PORT_SEG_AFF[NB_SEGMENTS]={40,41,42,43,44,45,46}; // segments de l'afficheur dans l'ordre A, B... G
const byte ETAT_SEG[9]={
  0b0111111,
  0b0000110,
  0b1011011,
  0b1001111,
  0b1100110,
  0b1101101,
  0b1111101,
  0b0000111,
  0b1111111};
  
Adafruit_NeoPixel barre_led_g = Adafruit_NeoPixel(LONGUEUR_BARRE_LED, PORT_BARRE_LED_G, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel barre_led_d = Adafruit_NeoPixel(LONGUEUR_BARRE_LED, PORT_BARRE_LED_D, NEO_GRB + NEO_KHZ800);
unsigned char led[2]={0,0}; // niveau des barres de led

Adafruit_NeoPixel led_porte[NB_PORTES];
Servo servo_porte[NB_PORTES];
Servo servo_voiture[NB_PORTES];

int solo = digitalRead(PORT_SWITCH);
unsigned char game_mode = 0; // décrémenté de 1 par rapport au pseudo-code (et à l'afficheur)
bool joueur = 0;


void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);

  pinMode(PORT_BOUTON, INPUT_PULLUP); // bouton par défaut HIGH, LOW quand appuyé

  barre_led_g.begin();
  barre_led_g.show();
  barre_led_d.begin();
  barre_led_d.show();


  unsigned char i;
  for (i=0 ; i<NB_PORTES ; i++)
  {
    led_porte[i]=Adafruit_NeoPixel(1, PORT_LED_PORTE[i], NEO_GRB + NEO_KHZ800);
    pinMode(PORT_BOUTON_PORTE[i], INPUT_PULLUP);
    led_porte[i].begin();
    led_porte[i].show();
    servo_porte[i].attach(PORT_SERVO_PORTE[i]);
    servo_voiture[i].attach(PORT_SERVO_VOITURE[i]);
    servo_porte[i].write(0); // par convention, 0 correspond à "porte fermée"
    servo_voiture[i].write(0); // par convention, 0 correspond à "on voit la chèvre"
  }

  for (i=0; i<NB_SEGMENTS; i++){
    pinMode(PORT_SEG_AFF[i], OUTPUT);
    digitalWrite(PORT_SEG_AFF[i], HIGH);
  }

  // initialisation de l'afficheur
  afficher(1);

  randomSeed(analogRead(0));

  Serial.println("setup fini"); Serial.flush();

}

void loop() {
  // put your main code here, to run repeatedly:

  Serial.println("loop commencee"); Serial.flush();
  unsigned long last_change=millis();
  while ((millis() - last_change) < (unsigned long) 5000)
  {
    delay(10);
    if (digitalRead(PORT_SWITCH) != solo) // (solo != solo=digitalRead(PORT_SWITCH))
    {
      last_change = millis();
      solo = digitalRead(PORT_SWITCH);
    }
    if (digitalRead(PORT_BOUTON) == LOW)
    {
      last_change = millis();
      game_mode++;
      game_mode%=4;
      afficher(game_mode+1 +4*(solo==0));
      delay(500);
    }
  }
  Serial.println("while fini"); Serial.flush();

  bool change, gagne;
  switch (game_mode)
  {
    case 0:
      Serial.println("case 0"); Serial.flush();
      while(led[0]!=LONGUEUR_BARRE_LED && led[1]!=LONGUEUR_BARRE_LED) {
        manche(3, 1, &change, &gagne);
        gerer_victoire(&change, &gagne);
      }
      break;
    case 1:
      Serial.println("case 1"); Serial.flush();
      while(led[0]!=LONGUEUR_BARRE_LED && led[1]!=LONGUEUR_BARRE_LED) {
        manche(5, 1, &change, &gagne);
        gerer_victoire(&change, &gagne);
      }
      break;
    case 2:
      Serial.println("case 2"); Serial.flush();
      while(led[0]!=LONGUEUR_BARRE_LED && led[1]!=LONGUEUR_BARRE_LED) {
        manche(5, 3, &change, &gagne);
        gerer_victoire(&change, &gagne);
      }
      break;
    case 3:
      Serial.println("case 3"); Serial.flush();
      unsigned char increment[2]={0,0};
      while(increment[0]<=2 && increment[1]<=2) {
        manche(3+increment[joueur], 1+increment[joueur], &change, &gagne);
        if(gagne) {increment[joueur]++;}
        gerer_victoire(&change, &gagne);
      }
      break;
  }
  Serial.println("switch fini"); Serial.flush();
  Serial.println("loop fini"); Serial.flush();
}

void manche(const unsigned int n, const unsigned int k, bool *change, bool *gagne) {
  Serial.println("manche commencee"); Serial.flush();
  bool contenu[NB_PORTES] = {0,0,0,0,0}; // 5 étant le nombre max de portes
  unsigned char etat[NB_PORTES] = {0,0,0,0,0}; // 0 fermé ; 1 ouvert ; 2 choisi (a) ; 3 choisi (b)
  long voiture = random(n);
  Serial.print("voiture choisie :");Serial.println(voiture); Serial.flush();
  contenu[voiture] = 1;

  unsigned long i,j,m;
  for(m=0 ; m++ ; m<NB_PORTES) {
    if(m==voiture) {
      servo_voiture[m].write(90); // par convention, 90° correspond à "on voit la voiture"
    }
    else {
      servo_voiture[m].write(0);
    }
  }

  Serial.println("choix voiture OK"); Serial.flush();

  choisir_porte(etat, &i, 2);
  
  Serial.println("choix porte 1 OK"); Serial.flush();

  long pres;
  for(m=0 ; m++ ; m<k) {
    do {
      pres = random(n);
    } while(etat[pres] || contenu[pres]) ;
    Serial.print("voiture choisie (présentateur) :") ; Serial.println(pres) ; Serial.flush();
    etat[pres] = 1;
    servo_porte[pres].write(90); // par convention, 90° correspond à "porte ouverte"
    delay(500); //attendre 500 ms
  }

  Serial.println("choix présentateur OK"); Serial.flush();

  choisir_porte(etat, &j, 3);

  Serial.println("choix porte 2 OK"); Serial.flush();
  
  servo_porte[j].write(90);

  *change = (bool) (i != j);
  *gagne = contenu[j];

  Serial.print("Change, gagne : ") ; Serial.print(*change) ; Serial.println(*gagne) ; Serial.flush();


  Serial.println("manche finie"); Serial.flush();
}

// fonction écrivant sur un seul afficheur
void afficher(char chiffre) {
    unsigned char m ;
  int segment = ETAT_SEG[chiffre];
//  Serial.println(segment);
  for (m=0; m<NB_SEGMENTS; m++){
    digitalWrite(PORT_SEG_AFF[m], !bitRead(ETAT_SEG[chiffre],m));
//    Serial.println( !bitRead(ETAT_SEG[chiffre],m)); Serial.flush();
  }
}
void choisir_porte(unsigned char etat[], unsigned long *i, char valeur) {
  bool choix_fait = false;
  unsigned long stub;
  while (!choix_fait) {
    for(stub=0 ; stub<NB_PORTES ; stub++) {
      int bouton_presse = digitalRead(PORT_BOUTON_PORTE[stub]);
      if(bouton_presse == HIGH) {
        Serial.print("Porte choisie (joueur) : ");Serial.println(stub); Serial.flush();
        etat[stub] = valeur;
        choix_fait = true;
        *i=stub;
        break;
      }
    }
   delay(10);
  }
}

void gerer_victoire(bool *change, bool *gagne) {
    if (solo) {
      if (*gagne) {
        led[*change] ++;
      }
    }
    else {
      if (*gagne) {
        led[joueur] ++;
      }
      joueur = 1 - joueur;
    }
  barre_led_g.setPixelColor(led[0], barre_led_g.Color(255, 255, 15));
  barre_led_d.setPixelColor(led[1], barre_led_d.Color(255, 255, 15));
}
