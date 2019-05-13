#include <FastLED.h>
#include <Servo.h>
#include "pitches.h"


// prototypes des fonctions
void manche(const unsigned int n, const unsigned int k,bool *change,bool *gagne);
void afficher(int chiffre);
void choisir_porte(unsigned int etat[], unsigned int *i, int valeur);
void gerer_victoire(bool *change, bool *gagne);
void jouer_melodie();
int get_random(int n);

// nombres d'or
#define PAUSE_PARAM 5000
#define NB_PORTES 5
#define MIN_ANGLE 80 // porte fermée / chèvre
#define MAX_ANGLE 5 // porte ouverte / voiture ; NE PAS METTRE 0 !
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
const int liste_random[50]={
  2, 0, 1, 4, 4, 1, 1, 1, 4, 4, 1, 1, 2, 3, 2, 4, 4, 2, 3, 3, 2, 0, 3, 0, 2,
  4, 4, 0, 0, 0, 0, 0, 4, 1, 1, 3, 1, 2, 0, 2, 1, 4, 3, 3, 2, 3, 3, 2, 0, 3
};
int index_random;

// brochages
#define PORT_BOUTON 14
#define PORT_SWITCH 15
#define PORT_BARRE_LED_HAUT 16
#define PORT_BARRE_LED_BAS 17
#define PORT_BUZZER 18
const unsigned int PORT_LED_PORTE[NB_PORTES]={4,6,8,10,12}; // fil blanc
const unsigned int PORT_BOUTON_PORTE[NB_PORTES]={5,7,9,11,13}; // fil jaune
const unsigned int PORT_SERVO_PORTE[NB_PORTES]={22,24,26,28,30};
const unsigned int PORT_SERVO_VOITURE[NB_PORTES]={23,25,27,29,31};
const unsigned int PORT_SEG_AFF[NB_SEGMENTS]={53,51,49,47,45,43,41}; // segments de l'afficheur dans l'ordre A, B... G

// variables globales
int led[2]; // niveau des barres de led
CRGB barre_led_haut[LONGUEUR_BARRE_LED];
CRGB barre_led_bas[LONGUEUR_BARRE_LED];
CRGB led_porte[NB_PORTES][1];
Servo servo_porte[NB_PORTES], servo_voiture[NB_PORTES];
int solo;
unsigned int game_mode; // décrémenté de 1 par rapport au pseudo-code (et à l'afficheur)
bool joueur;


void setup() {
  // put your setup code here, to run once:

  pinMode(PORT_BOUTON, INPUT_PULLUP); // bouton par défaut HIGH, LOW quand appuyé
  pinMode(PORT_SWITCH, INPUT); // par défaut LOW, HIGH quand activé
  pinMode(PORT_BUZZER, OUTPUT);
  FastLED.addLeds<NEOPIXEL, PORT_BARRE_LED_HAUT>(barre_led_haut, LONGUEUR_BARRE_LED);
  FastLED.addLeds<NEOPIXEL, PORT_BARRE_LED_BAS>(barre_led_bas, LONGUEUR_BARRE_LED);
  FastLED.addLeds<NEOPIXEL, 4>(led_porte[0], 1);
  FastLED.addLeds<NEOPIXEL, 6>(led_porte[1], 1);
  FastLED.addLeds<NEOPIXEL, 8>(led_porte[2], 1);
  FastLED.addLeds<NEOPIXEL, 10>(led_porte[3], 1);
  FastLED.addLeds<NEOPIXEL, 12>(led_porte[4], 1);
  FastLED.setBrightness(128);
  jouer_melodie();

  game_mode = 0;
  joueur = 0;
  led[0] = led[1] = 0;
  solo = 1 - digitalRead(PORT_SWITCH); // 0 si mode multi-joueurs

  unsigned int i;
  for (i=0; i<LONGUEUR_BARRE_LED; i++)
  {
    barre_led_bas[i] = CRGB::Black;
    barre_led_haut[i] = CRGB::Black;
    FastLED.show();
  }
  for (i=0 ; i<NB_PORTES ; i++) // initialisation de tout ce qui est associé à une porte
  {
    led_porte[i][0] = CRGB::Black;
    FastLED.show();
    delay(200);
    pinMode(PORT_SERVO_VOITURE[i], OUTPUT);
    pinMode(PORT_SERVO_PORTE[i], OUTPUT);
    pinMode(PORT_LED_PORTE[i], OUTPUT);
    pinMode(PORT_BOUTON_PORTE[i], INPUT);
    servo_porte[i].attach(PORT_SERVO_PORTE[i]);
    servo_voiture[i].attach(PORT_SERVO_VOITURE[i]);
    servo_porte[i].write(MIN_ANGLE);
    servo_voiture[i].write(MIN_ANGLE);
    delay(500); // attendre les servos
    servo_porte[i].detach();
    servo_voiture[i].detach();
  }

  // initialisation de l'afficheur
  for (i=0; i<NB_SEGMENTS; i++){
    pinMode(PORT_SEG_AFF[i], OUTPUT);
    digitalWrite(PORT_SEG_AFF[i], HIGH);
  }
  afficher(1);

  // initialisation de get_random
  index_random = analogRead(0);


  // récupération des paramètres de jeu : mode de jeu + nombre de joueurs
  unsigned long last_change=millis();
  // on attend qu'il n'y ait pas eu d'interaction pendant 'PAUSE_PARAM's avant de valider les paramètres de jeu
  delay(50); // attendre 50ms pour que millis() devienne différent de last_change ?
  while ((millis() - last_change) < PAUSE_PARAM)
  {
    if (1-digitalRead(PORT_SWITCH) != solo)
    {
      last_change = millis();
      solo = 1 - digitalRead(PORT_SWITCH); 
    }
    if (digitalRead(PORT_BOUTON) == LOW)
    {
      last_change = millis();
      game_mode++;
      game_mode%=4;
      afficher(game_mode+1); // on affiche de 1 à 4, pas de 0 à 3
    }
    delay(500); // il ne faut pas boucler "dans le vide" sinon la carte reset
    // + premettre au joueur de relâcher le bouton
  }


  bool change, gagne;
  switch (game_mode) // lancer le bon mode de jeu
  {
    case 0:
      while(led[0]!=LONGUEUR_BARRE_LED && led[1]!=LONGUEUR_BARRE_LED) {
        manche(3, 1, &change, &gagne);
        gerer_victoire(&change, &gagne);
        delay(1000);
      }
      break;
    case 1:
      while(led[0]!=LONGUEUR_BARRE_LED && led[1]!=LONGUEUR_BARRE_LED) {
        manche(5, 1, &change, &gagne);
        gerer_victoire(&change, &gagne);
        delay(1000);
      }
      break;
    case 2:
      while(led[0]!=LONGUEUR_BARRE_LED && led[1]!=LONGUEUR_BARRE_LED) {
        manche(5, 3, &change, &gagne);
        gerer_victoire(&change, &gagne);
        delay(1000);
      }
      break;
    case 3:
      unsigned int increment[2]={0,0}; // représente le nombre de portes rajoutées à chaque joueur
      while(increment[0]<=2 && increment[1]<=2) {
        manche(3+increment[joueur], 1+increment[joueur], &change, &gagne);
        if(gagne) {increment[joueur]++;}
        gerer_victoire(&change, &gagne);
        delay(1000);
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
  unsigned int etat[NB_PORTES] = {0}; // 0 fermé ; 1 ouvert ; 2 choisi (a) ; 3 choisi (b)
  int voiture = get_random(n);
  contenu[voiture] = 1;

  unsigned int i,j,m; // i,j choix du joueur ; m variable d'incrémentation
  
  for (m=0; m<n; m++) {
    led_porte[m][0] = CRGB::Black; // on éteint les leds de chaque porte avant de commencer
    FastLED.show();
    servo_porte[m].attach(PORT_SERVO_PORTE[m]);
    servo_porte[m].write(MIN_ANGLE);
    delay(500);
    servo_porte[m].detach();
    if(m==voiture) {
      servo_voiture[m].attach(PORT_SERVO_VOITURE[m]);
      servo_voiture[m].write(MAX_ANGLE);
      delay(500);
      servo_voiture[m].detach();
    }
    else {
      servo_voiture[m].attach(PORT_SERVO_VOITURE[m]);
      servo_voiture[m].write(MIN_ANGLE);
      delay(500);
      servo_voiture[m].detach();
          }
    }
    
  for (m=n; m<NB_PORTES; m++){
    servo_porte[m].attach(PORT_SERVO_PORTE[m]);
    servo_porte[m].write(MAX_ANGLE);
    delay(500);
    servo_porte[m].detach();
    etat[m] = 1;
  }
  
  choisir_porte(etat, &i, 2);

  delay(1000);

  int pres;
  for(m=0 ; m<k; m++) { // on choisit k pourtes à ouvrir par le présentateur
    do {
      pres = get_random(n);
    } while(etat[pres] || contenu[pres]) ; // la porte doit contenir une chèvre et ne pas avoir déjà été choisie
    
    etat[pres] = 1;
    led_porte[pres][0] = CRGB::Blue;
    FastLED.show();
    servo_porte[pres].attach(PORT_SERVO_PORTE[pres]);
    servo_porte[pres].write(MAX_ANGLE);
    delay(500); //attendre le servo pendant 1000 ms
    servo_porte[pres].detach();
  }
  
  choisir_porte(etat, &j, 3);

  delay(1000);
  servo_porte[j].attach(PORT_SERVO_PORTE[j]);
  servo_porte[j].write(MAX_ANGLE); // on ouvre la 2e porte choisie par le joueur
  delay(500);
  servo_porte[j].detach();
  *change = (bool) (i != j);
  *gagne = contenu[j];

  if (*gagne)
  {
    led_porte[j][0] = CRGB::Green;
    FastLED.show();
  }
  else
  {
    led_porte[j][0] = CRGB::Red;
    FastLED.show();
  }
}

void afficher(int chiffre) {
  /* fonction écrivant sur un seul afficheur */
  unsigned int m ;
  int segment = ETAT_SEG[chiffre];
  for (m=0; m<NB_SEGMENTS; m++){
    digitalWrite(PORT_SEG_AFF[m], !bitRead(ETAT_SEG[chiffre],m)); // bitRead :
  }
}

void choisir_porte(unsigned int etat[], unsigned int *i, int valeur) {
  bool choix_fait = false;
  unsigned long stub;
  while (!choix_fait) {
    for(stub=0 ; stub<NB_PORTES ; stub++) { // /!\ pas de pointeur dans le for !
      int bouton_presse = digitalRead(PORT_BOUTON_PORTE[stub]);
      if (bouton_presse == HIGH && etat[stub]!=1) { 
        etat[stub] = valeur;
        led_porte[stub][0] = CRGB::Yellow;
        FastLED.show();
        choix_fait = true;
        *i=stub;
        break;
      }
    }
   delay(100);
  }
}


void gerer_victoire(bool *change, bool *gagne) {
  /* fonction gérant l'allumage des leds et le changement de joueurs en multijoueur ; appelée à la fin d'une manche
   *  (même si la manche a été perdue)
   */
    if (solo) {
      if (*gagne) {
        led[*change] ++;
        if (led[0]>0){
        barre_led_haut[led[0]-1] = CRGB::Yellow;}
        if(led[1]>0){
        barre_led_bas[led[1]-1] = CRGB::Yellow;}
        FastLED.show();
        jouer_melodie();
      }
    }
    else {
      if (*gagne) {
        led[joueur] ++;
        if (led[0]>0){
        barre_led_haut[led[0]-1] = CRGB::Yellow;}
        if (led[1]>0){
        barre_led_bas[led[1]-1] = CRGB::Yellow;}
        FastLED.show();
        jouer_melodie();
      }
      joueur = 1 - joueur;
    }
  delay(2000);
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

int get_random(int n)
{
  do { index_random++;index_random %= 50;} while (liste_random[index_random] >= n);
  return liste_random[index_random];
}
