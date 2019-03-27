from time import sleep, clock
from random import randrange

# valeur_bouton | nombre de portes | nombre de portes enlevées
# ____________________________________________________________
# 1				| 3					| 1
# 2				| 5					| 1
# 3				| 5					| 3
# 4				| 3->5				| 1->3
# 5				|idem mais multi-only
# 6				|idem mais multi-only
# 7				|idem mais multi-only
# 8				|idem mais multi-only


def init():
    solo = get_switch()
    game_mode = 1
    joueur = 0

def loop():

    # récupérer les entrées pour la séquence de jeu (solo, game_mode)

    last_change = clock()
    while last_change - clock() < 3 :
    	sleep(0.1)
        if get_switch() != solo :
            last_change = clock()
            solo = get_switch()
        if bouton_presse() : # si le bouton a été pressé depuis le dernier appel à bouton_presse
            last_change = clock()
            game_mode += 1
            if game_mode > 4 :
                game_mode -= 4

        afficher_game_mode(game_mode + 4*(not solo))

    # jouer

    led = [0,0] #niveaux des barres de LED

    if game_mode == 1 :
        while 10 not in led :
            changé, gagné = manche(3,1)
            if solo :
                if gagné :
                    led[changé] += 1
            else :
                if gagné :
                    led[joueur] += 1
                joueur = 1-joueur
            afficher_led(led)
    if game_mode == 2 :
        while 10 not in led :
            changé, gagné = manche(5,1)
            if solo :
                if gagné :
                    led[changé] += 1
            else :
                if gagné :
                    led[joueur] += 1
                joueur = 1-joueur
            afficher_led(led)
    if game_mode == 3 :
        while 10 not in led :
            changé, gagné = manche(5,3)
            if solo :
                if gagné :
                    led[changé] += 1
            else :
                if gagné :
                    led[joueur] += 1
                joueur = 1-joueur
            afficher_led(led)
    if game_mode == 4 :
        incrément = [0,0]
        while 10 not in led :
            changé, gagné = manche(3+incrément[joueur], 5+incrément[joueur])
            if solo :
                if gagné :
                    led[changé] += 1 # à changer pour remplir en 3 victoires
                    incrément[joueur] += 1
            else :
                if gagné :
                    led[joueur] += 1
                    incrément[joueur] += 1
                joueur = 1-joueur
            afficher_led(led)

def manche(n, k):
    """n portes, k à ouvrir"""

    # initialisation
    contenu = [0,]*n
    voiture = randrange(n)
    contenu[voiture] = 1
    placer_voiture(voiture)
    état = [0,]*n # 0 fermé ; 1 ouvert ; 2 choisi (a) ; 3 choisi (b)

    while 2 not in état :
        for i in range(n) :
            btn = get_bouton_porte(i)
            if btn :
                état[i] = 2
                break
        sleep(0.1)

    for _ in range(k) :
        pres = randrange(n)
        while état[pres] or contenu[pres] :
            pres = randrange(n)
        état[pres] = 1
        ouvrir_porte(pres)
        # sleep(0.5)

    while 3 not in état :
        for j in range(n) :
            btn = get_bouton_porte(j)
            if btn :
                état[j] = 3
                break
        sleep(0.1)

    changé = (i != j)
    gagné = contenu[j]
    return (changé, gagné)
