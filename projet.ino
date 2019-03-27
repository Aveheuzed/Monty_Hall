#include <Adafruit_NeoPixel.h>
#include <Servo.h>

#define PORT_BOUTON 2
#define PORT_SWITCH 3
#define PORT_BARRE_LED_G 4
#define PORT_BARRE_LED_D 5
#define PORT_BUZZER 6

#define NB_PORTES 5
#define LONGUEUR_BARRE_LED 10

const unsigned char PORT_LED_PORTE[NB_PORTES]={7,8,9,10,11,12};
const unsigned char PORT_BOUTON_PORTE[NB_PORTES]={14,15,16,17,18,19};
const unsigned char PORT_SERVO_PORTE[NB_PORTES]={20,21,22,23,24,25,26};
const unsigned char PORT_SERVO_VOITURE[NB_PORTES]={27,28,29,30,31,32};
const unsigned char AFFICHEUR[4]={33,34,35,36}; // on n'aura besoin que des 2 bits de poids faible (33,34)

Adafruit_NeoPixel barre_led_g = Adafruit_NeoPixel(LONGUEUR_BARRE_LED, PORT_BARRE_LED_G, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel barre_led_d = Adafruit_NeoPixel(LONGUEUR_BARRE_LED, PORT_BARRE_LED_D, NEO_GRB + NEO_KHZ800);

Adafruit_NeoPixel led_porte[NB_PORTES];
Servo servo_porte[NB_PORTES];
Servo servo_voiture[NB_PORTES];

unsigned char solo = digitalRead(PORT_SWITCH);
unsigned char game_mode = 0; // décrémenté de 1 par rapport au pseudo-code (et à l'afficheur)
bool joueur = 0;



void setup() {
  // put your setup code here, to run once:

  pinMode(PORT_BOUTON, INPUT_PULLUP) // bouton par défaut HIGH, LOW quand appuyé

  barre_led_g.begin();
  barre_led_g.show();
  barre_led_d.begin();
  barre_led_d.show();


  unsigned char i;
  for (i=0 ; i<NB_PORTES ; i++)
  {
    led_porte[i]=Adafruit_NeoPixel(1, PORT_LED_PORTE[i], NEO_GRB + NEO_KHZ800);
    led_porte[i].begin();
    led_porte[i].show();
    servo_porte[i].attach(PORT_SERVO_PORTE[i]);
    servo_voiture[i].attach(PORT_SERVO_VOITURE[i]);
    servo_porte[i].write(0); // par convention, 0 correspond à "porte fermée"
    servo_voiture[i].write(0); // par convention, 0 correspond à "on voit la chèvre"
  }

  // initialisation de l'afficheur
  for (i=0 ; i<4 ; i++)
  {
    digitalWrite(AFFICHEUR[i], LOW);
  }
}



void loop() {
  // put your main code here, to run repeatedly:
  unsigned long last_change=millis();
  while (last_change - millis() < 3000)
  {
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
      afficher(game_mode+1 +4*!(bool solo))
    }

    char led[2]={0,0};

    switch (game_mode)
    {
      case 0:
        break;
      case 1:
        break;
      case 2:
        break;
      case 3:
        break;
    }
  }

}



// fonction écrivant sur un seul afficheur
void afficher(char chiffre)
{
    // On allume les bits nécessaires
    if(chiffre >= 2)
    {
      digitalWrite(AFFICHEUR[1], HIGH);
        chiffre = chiffre - 2;
    }
    if(chiffre >= 1)
    {
        digitalWrite(AFFICHEUR[0], HIGH);
        chiffre = chiffre - 1;
    }
}
