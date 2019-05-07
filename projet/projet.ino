#include <Adafruit_NeoPixel.h>
#include <Servo.h>
#include "pitches.h"

// prototypes des fonctions
void manche(const unsigned int n, const unsigned int k, bool *change, bool *gagne);
void afficher(char chiffre);
void choisir_porte(unsigned char etat[], unsigned long int *i, char valeur);
void gerer_victoire(bool *change, bool *gagne);
void jouer_melodie();

// nombres d'or
#define NB_PORTES 5
#define MIN_ANGLE 0 // porte fermée / chèvre
#define MAX_ANGLE 80 // porte ouverte / voiture
#define LONGUEUR_BARRE_LED 12
#define NB_SEGMENTS 7
#define DUREE_MELODIE 7
const int melody[DUREE_MELODIE] = {NOTE_C4, NOTE_E4, NOTE_G4,0, NOTE_E4, NOTE_C4, NOTE_G4};
const int noteDurations[DUREE_MELODIE] = {8, 8, 8, 8, 8, 8, 8};
const byte ETAT_SEG[10]={
  0b0111111,
  0b0000110,
  0b1011011,
  0b1001111,
  0b1100110,
  0b1101101,
  0b1111101,
  0b0000111,
  0b1111111,
  0b1111011}; // représentation des chiffres de 0 à 9 sur un afficheur 7 segments

// brochages
#define PORT_BOUTON 14
#define PORT_SWITCH 15
#define PORT_BARRE_LED_HAUT 16
#define PORT_BARRE_LED_BAS 17
#define PORT_BUZZER 18
const unsigned char PORT_LED_PORTE[NB_PORTES]={4,6,8,10,12}; // fil blanc
const unsigned char PORT_BOUTON_PORTE[NB_PORTES]={5,7,9,11,13}; // fil jaune
const unsigned char PORT_SERVO_PORTE[NB_PORTES]={22,24,26,28,30};
const unsigned char PORT_SERVO_VOITURE[NB_PORTES]={23,25,27,29,31};
const unsigned char PORT_SEG_AFF[NB_SEGMENTS]={40,41,42,43,44,45,46}; // segments de l'afficheur dans l'ordre A, B... G

// variables globales
unsigned char led[2]; // niveau des barres de led
Adafruit_NeoPixel barre_led_haut, barre_led_bas, led_porte[NB_PORTES];
Servo servo_porte[NB_PORTES], servo_voiture[NB_PORTES];
int solo;
unsigned char game_mode; // décrémenté de 1 par rapport au pseudo-code (et à l'afficheur)
bool joueur;


void setup() {
  // put your setup code here, to run once:

  pinMode(PORT_BOUTON, INPUT_PULLUP); // bouton par défaut HIGH, LOW quand appuyé
  pinMode(PORT_SWITCH, INPUT); // par défaut LOW, HIGH quand activé

  game_mode = 0;
  joueur = 0;
  led[0] = led[1] = 0;
  barre_led_haut = Adafruit_NeoPixel(LONGUEUR_BARRE_LED, PORT_BARRE_LED_HAUT, NEO_GRB + NEO_KHZ800);
  barre_led_bas = Adafruit_NeoPixel(LONGUEUR_BARRE_LED, PORT_BARRE_LED_BAS, NEO_GRB + NEO_KHZ800);

  solo = digitalRead(PORT_SWITCH);


  barre_led_haut.begin(); // initialisation des 2 barres de led
  barre_led_haut.show();
  barre_led_bas.begin();
  barre_led_bas.show();

  unsigned char i;
  for (i=0 ; i<NB_PORTES ; i++) // initialisation de tout ce qui est associé à une porte
  {
    pinMode(PORT_BOUTON_PORTE[i], INPUT_PULLUP);
    led_porte[i]=Adafruit_NeoPixel(1, PORT_LED_PORTE[i], NEO_GRB + NEO_KHZ800);
    led_porte[i].begin();
    led_porte[i].show();
    servo_porte[i].attach(PORT_SERVO_PORTE[i]);
    servo_voiture[i].attach(PORT_SERVO_VOITURE[i]);
    servo_porte[i].write(MIN_ANGLE);
    servo_voiture[i].write(MIN_ANGLE);
  }

  // initialisation de l'afficheur
  for (i=0; i<NB_SEGMENTS; i++){
    pinMode(PORT_SEG_AFF[i], OUTPUT);
    digitalWrite(PORT_SEG_AFF[i], HIGH);
  }
  afficher(1);

  randomSeed(analogRead(0)); // initialisation du random




  // récupération des paramètres de jeu : mode de jeu + nombre de joueurs
  unsigned long last_change=millis();
  // on attend qu'il n'y ait pas eu d'interaction pendant 3s avant de valider les paramères de jeu
  delay(50); // attendre 50ms pour que millis() devienne différent de last_change ?
  while ((millis() - last_change) < 3000)
  {
    if (digitalRead(PORT_SWITCH) != solo)
    {
      last_change = millis();
      solo = digitalRead(PORT_SWITCH);
    }
    if (digitalRead(PORT_BOUTON) == LOW)
    {
      last_change = millis();
      game_mode++;
      game_mode%=4;
      afficher(game_mode+1); // on affiche de 1 à 4, pas de 0 à 3
    }
    delay(50); // il ne faut pas boucler "dans le vide" sinon la carte reset
  }

  bool change, gagne;
  switch (game_mode) // lancer le bon mode de jeu
  {
    case 0:
      while(led[0]!=LONGUEUR_BARRE_LED && led[1]!=LONGUEUR_BARRE_LED) {
        manche(3, 1, &change, &gagne);
        gerer_victoire(&change, &gagne);
      }
      break;
    case 1:
      while(led[0]!=LONGUEUR_BARRE_LED && led[1]!=LONGUEUR_BARRE_LED) {
        manche(5, 1, &change, &gagne);
        gerer_victoire(&change, &gagne);
      }
      break;
    case 2:
      while(led[0]!=LONGUEUR_BARRE_LED && led[1]!=LONGUEUR_BARRE_LED) {
        manche(5, 3, &change, &gagne);
        gerer_victoire(&change, &gagne);
      }
      break;
    case 3:
      unsigned char increment[2]={0,0}; // représente le nombre de portes rajoutées à chaque joueur
      while(increment[0]<=2 && increment[1]<=2) {
        manche(3+increment[joueur], 1+increment[joueur], &change, &gagne);
        if(gagne) {increment[joueur]++;}
        gerer_victoire(&change, &gagne);
      }
      break;
  }
}


void loop() {}

void manche(const unsigned int n, const unsigned int k, bool *change, bool *gagne) {
  /*
   * gestion d'une seule manche
   */
  bool contenu[NB_PORTES] = {0}; // ce tableau est initialisé à 0 pour chaque indice
  unsigned char etat[NB_PORTES] = {0}; // 0 fermé ; 1 ouvert ; 2 choisi (a) ; 3 choisi (b)
  int voiture = (int) random((long) n);
  contenu[voiture] = 1;

  unsigned int i,j,m; // i,j choix du joueur ; m variable d'incrémentation
  for (m=0 ; m++ ; m<NB_PORTES) {
    allumer_led(led_porte[m], 0, 0, 0, 0); // on éteint les leds de chaque porte avant de commencer
    if(m==voiture) {
      servo_voiture[m].write(MAX_ANGLE);
    }
    else {
      servo_voiture[m].write(MIN_ANGLE);
    }
  }
  choisir_porte(etat, &i, 2);

  int pres;
  for(m=0 ; m++ ; m<k) { // on choisit k pourtes à ouvrir par le présentateur
    do {
      pres = (int) random((long) n);
    } while(etat[pres] || contenu[pres]) ; // la porte doit contenir une chèvre et ne pas avoir déjà été choisie
    etat[pres] = 1;
    allumer_led(led_porte[pres], 0 , 0, 0, 127)
    servo_porte[pres].write(MAX_ANGLE);
    delay(500); //attendre le servo pendant 500 ms
  }

  choisir_porte(etat, &j, 3);

  servo_porte[j].write(MAX_ANGLE); // on ouvre la 2e porte choisie par le joueur

  *change = (bool) (i != j);
  *gagne = contenu[j];

  if (*gagne)
  {
    allumer_led(led_porte[j], 0, 0, 127, 0);
  }
  else
  {
    allumer_led(led_porte[j], 0, 127, 0, 0);
  }
}

void afficher(char chiffre) {
  /* fonction écrivant sur un seul afficheur */
  unsigned char m ;
  int segment = ETAT_SEG[chiffre];
  for (m=0; m<NB_SEGMENTS; m++){
    digitalWrite(PORT_SEG_AFF[m], !bitRead(ETAT_SEG[chiffre],m)); // bitRead :
  }
}

void choisir_porte(unsigned char etat[], unsigned long *i, char valeur) {
  bool choix_fait = false;
  unsigned long stub;
  while (!choix_fait) {
    for(stub=0 ; stub<NB_PORTES ; stub++) { // /!\ pas de pointeur dans le for !
      int bouton_presse = digitalRead(PORT_BOUTON_PORTE[stub]);
      if (bouton_presse == LOW) {
        etat[stub] = valeur;
        allumer_led(led_porte[stub], 0, 127, 127, 0);
        choix_fait = true;
        *i=stub;
        break;
      }
    }
   delay(10);
  }
}

void gerer_victoire(bool *change, bool *gagne) {
  /* fonction gérant l'allumage des leds et le changement de joueurs en multijoueur ; appelée à la fin d'une manche
   *  (même si la manche a été pardue)
   */
    if (solo) {
      if (*gagne) {
        led[*change] ++;
        jouer_melodie();
      }
    }
    else {
      if (*gagne) {
        led[joueur] ++;
        jouer_melodie();
      }
      joueur = 1 - joueur;
    }
  // on actualise la led la plus haute de la barre / on en allume une de plus
  allumer_led(barre_led_haut, led[0], 255, 255, 15);
  allumer_led(barre_led_bas, led[1], 255, 255, 15);
}

void jouer_melodie() {
  /*
   *  joue la mélodie de victoire
   *  copié de http://www.arduino.cc/en/Tutorial/Tone
   *  created 21 Jan 2010
   *  modified 30 Aug 2011
   *  by Tom Igoe
   *  This example code is in the public domain.
   */
   const int melody[] = {NOTE_C4, NOTE_E4, NOTE_G4,0, NOTE_E4, NOTE_C4, NOTE_G4};
   const int noteDurations[] = {8, 8, 8, 8, 8, 8, 8};
   for (int thisNote = 0; thisNote < DUREE_MELODIE; thisNote++)
   {
    // to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(PORT_BUZZER, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing
    noTone(PORT_BUZZER);
  }
}

void allumer_led(Adafruit_NeoPixel ruban, int index, int r, int v, int b) {
  ruban.setPixelColor(index, ruban.Color(r,v,b))
  ruban.show()
}
