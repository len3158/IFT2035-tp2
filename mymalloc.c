/*
    Auteurs : Mehran ASADI 1047847 
              Lenny SIEMENI 1055234 
*/
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include "mymalloc.h"
#define MAXSIZE 16711568
#define ALIGN(size) (((size) + (16-1)) & ~(16-1))
#define CASEALLIGNE ALIGN(sizeof(struct cases))
#define TAILLEPAGE ALIGN(4096) + CASEALLIGNE
#define ALIGNPAGE(size) (((size) + (4096-1)) & ~(4096-1))
struct cases{
    size_t taille, taillePage;
    int compt;
    struct cases *suiv, *prec, *pageSuiv, *libre;
};
struct navigateurs{
    struct cases *debut, *fin, *finPage, *debutLibre;
};
typedef struct cases *Case;
typedef struct navigateurs *Navigateurs;
void initNavigateur(); ///Fonction qui créer la premiere page memoire avec un navigateur
Case pageVideGenerateur(size_t taille); ///Fonction qui génere une page memoir
Case memoireVide(size_t taille); ///Fonction qui retourne une case libre d'au moins la taille demandé
void trancheCase(Case caseP, size_t taille); ///Fonction qui tranche la case memoire à la taille demander
Case trouverCase(void *ptr); ///Fonction qui retourne la case memoire via son pointeur
void fusionner(Case actuel); ///Fonction qui funsionne deux cases libre cote-a-cote dans la meme page memoire
void detruirepage(Case actuel); ///Fonction qui détruit la page libre si c'est la premiere page de la liste ou la seul page restante
void imprimer(); ///Imprimer la liste des cases memoires situé dans chacun des pages memoires
void imprimervide(); ///Imprimer la liste des cases memoire libre

///Statique
static Navigateurs nav = NULL;
///Fonction qui créer la premiere page memoire avec un navigateur
void initNavigateur(){
    ///Alloue un peu de mémoire pour le navigateur
    nav = malloc(ALIGN(sizeof(struct navigateurs)));
    if(!nav){
        printf("probleme d'allocation memoire");
        return;
    }
    ///Creer un nouveau case vide
    Case caseVide = pageVideGenerateur(TAILLEPAGE);
    ///Pointe les éléments du Navigateur vers le nouveau block
    nav->debut = caseVide;       ///Premier case
    nav->fin = caseVide;        ///Dernier case
    nav->debutLibre = caseVide;   ///Premier case libre
    nav->finPage = caseVide;    ///dernier page
}

Case pageVideGenerateur(size_t taille){
    Case caseVide = malloc(taille);
    if(!caseVide){
        printf("probleme d'allocation memoire");
        return NULL;
    }
    caseVide ->taille = taille - CASEALLIGNE;   ///Taille de memoire
    caseVide ->taillePage = caseVide->taille;   ///taille de la page memoire
    caseVide ->compt = 0;                  ///Compteur de reference
    caseVide ->suiv = NULL;                ///case suivant
    caseVide ->prec = NULL;                ///case pécédent
    caseVide ->libre = NULL;                ///prochain case libre
    caseVide ->pageSuiv = NULL;            ///Prochaine page memoire
    return caseVide;

}
///Fonction qui retourne une case libre d'au moins la taille demandé
Case memoireVide(size_t taille){
    Case prec = NULL;
    Case actuel = nav->debutLibre;
    while(actuel != NULL){
        ///Si la taille n'est pas assez, on passe à la case libre suivant
        if(actuel->taille < taille){
            prec = actuel;
            actuel = actuel->libre;
        }
            ///Sinon on enleve la case de la list vide
        else{
            if(prec) ///si la case libre precedent existe, alors arranger les pointeurs
                prec->libre = actuel->libre;
            else ///sinon on l'ajoute en premier dans la liste case libre
                nav->debutLibre = actuel->libre;
            ///Enlever la case de la liste des cases libre
            actuel->libre = NULL;
            return actuel;
        }
    }
    ///Aucun memoire trouvé avec la taille demandé,
    ///donc on creer une nouvelle page memoire
    int tailleP = TAILLEPAGE;
    ///Si la taille demandee est plus grand, on agrandit la taille
    if(taille > TAILLEPAGE - CASEALLIGNE)
        tailleP = ALIGNPAGE(taille) + CASEALLIGNE; ///aligner la page memoire
    Case caseVide = pageVideGenerateur(tailleP); ///Generer une page memoire vide
    ///l'ajouter à la fin de la liste des cases et pages
    caseVide->prec = nav->fin;
    nav->fin->suiv = caseVide;
    nav->fin = caseVide;
    nav->finPage->pageSuiv = caseVide;
    nav->finPage = caseVide;
    return caseVide;
}
///Fonction qui retourne la case memoire via son pointeur
Case trouverCase(void *ptr){
    ///Verifier si le pointeur est null
    if(!ptr){
        printf("Erreur trouverCase(): Pointeur null\n");
        return NULL;
    }
    ///Chercher la page memoire contenant le pointeur
    void* addrptr = (void*) (ptr) - sizeof(struct cases);
    Case caseptr = (Case)addrptr;
    if(!caseptr)
        return NULL;
    else
        return caseptr;
}
///Fonction qui tranche la case memoire à la taille demander
void trancheCase(Case caseP, size_t taille){

    Case caseVide = ((void*) (caseP+1)) + taille; ///Trancher la case memoire
    ///Rearranger les pointeurs
    if(caseP->suiv)
        caseP->suiv->prec = caseVide;
    caseVide->suiv = caseP->suiv;
    caseVide->prec = caseP;
    caseP->suiv = caseVide;
    ///Changer les valeur des attributs
    caseVide->libre = NULL;
    caseVide->compt = 0;
    caseVide->taille = caseP->taille - CASEALLIGNE - taille; ///Définier la taille de la nouvelle case libre
    caseP->taille = taille; ///Redefinitir la taille de la case tranché

    ///Ajouter la nouvelle case libre dans la liste libre
    if(!nav->debutLibre){ ///Si aucun case libre, ajouter au premier la liste libre
        nav->debutLibre = caseVide;
    }
    else{
        void* addrActuel = (void*) caseVide;
        void* addrNav = (void*) nav->debutLibre;
        if(addrActuel < addrNav){ ///Si aucun case libre precedent, ajouter au debut de la liste libre
            caseVide->libre = nav->debutLibre;
            nav->debutLibre = caseVide;
        }else{ ///Sinon trouver une case libre precedent le plus proche puis rearranger les pointeurs
            Case casePrecVide = nav->debutLibre;
            void* addrLibrePrec = (void*) casePrecVide;
            while(addrActuel>addrLibrePrec){
                if(!casePrecVide->libre){
                    break;
                }
                casePrecVide = casePrecVide->libre;
                addrLibrePrec = (void*) casePrecVide;
            }
            caseVide->libre = casePrecVide->libre;
            casePrecVide->libre = caseVide;
        }
    }

    ///Si la nouvelle case est le dernier, l'ajouter a la fin de la liste
    if(caseVide->suiv == NULL){
        nav->fin->suiv = caseVide;
        nav->fin = caseVide;
    }
}
///Fonction qui funsionne deux cases libre cote-a-cote dans la meme page memoire
void fusionner(Case actuel){
    Case caseSuiv = actuel->suiv;
    ///Tant que la case suivante, dans la meme page, est libre, on les fusionne ensemble
    while(caseSuiv && caseSuiv->compt == 0 && caseSuiv->pageSuiv == NULL && caseSuiv != nav->finPage){
        Case caseSuive2 = caseSuiv->suiv;
        ///Si la case suivante est le dernier, alors on ajoute la case fusionné la la fin de la liste
        if(!caseSuive2){
            nav->fin = actuel;
            caseSuiv->prec = NULL;
            actuel->suiv = NULL;
            actuel->taille = (actuel->taille) + (caseSuiv->taille) + CASEALLIGNE;
            actuel->libre = caseSuiv->libre;
            ///Sinon rearranger les pointeurs de la nouvelle case fusionné
        }else{
            actuel->suiv = caseSuive2;
            caseSuive2->prec = actuel;
            actuel->taille = actuel->taille + caseSuiv->taille + CASEALLIGNE;
            caseSuiv->suiv = NULL;
            caseSuiv->prec = NULL;
            actuel->libre = caseSuiv->libre;
        }
        caseSuiv = actuel->suiv;
    }
    return;
}
///Imprimer la liste des cases memoires situé dans chacun des pages memoires
void imprimer(){
    printf("La structure du memoire: \n");
    Case actuel = nav->debut;
    Case pageSuiv = nav->debut->pageSuiv;
    printf("{{ TaillePage = %d ",actuel->taillePage);
    while(actuel){
        if(pageSuiv && actuel == pageSuiv){
            printf("}} \n   -> {{ TaillePage = %d ",actuel->taillePage);
            actuel = pageSuiv;
            pageSuiv = actuel->pageSuiv;
        }
        printf(" | (size: %d) (compteur: %d) ",actuel->taille,actuel->compt);
        if(actuel->compt == 0)
            printf("(Libre) | ");
        else
            printf("(Non libre)  | ");
        actuel = actuel->suiv;
        if(actuel && actuel != pageSuiv)
            printf("-> ");

    }
    printf(" }}\n");
}
///Imprimer la liste des cases memoire libre
void imprimervide(){
    printf("Liste de memoire libre :");
    Case actuel = nav->debutLibre;
    while(actuel){
        printf(" | (size: %d) |->",actuel->taille);
        actuel = actuel->libre;
    }
    printf("\n\n");
}

///Fonction qui retourne une case de memoire libre avec au moins la taille demander
void *mymalloc(size_t taille){
    /// Retourner NULL si la taille est null ou negatif
    //int tmp = (int)taille;
    if((int)taille <= 0){
        printf("Erreur mymalloc(): Taille null ou negative\n");
        return NULL;
    }
    if(taille > MAXSIZE){
        printf("Erreur mymalloc(): Depasse taille maximum 16Mo\n");
        return NULL;
    }
    ///Si navigateur est null, alors il faut creer la liste
    if(nav == NULL)
        initNavigateur();
    ///Aligner la taille memoire sur un multiple de 16 octet
    taille = ALIGN(taille);
    ///Trouver une case memoire libre avec au moins la taille demandee
    Case blockInsert = memoireVide(taille);

    ///On incrementer le compteur de reference
    ///refinc(blockInsert+1);
    blockInsert->compt++;

    ///Si la taille trouvé est plus grand que la taille demandé
    ///alors on tranche la case memoire
    if(blockInsert->taille > taille)
        trancheCase(blockInsert, taille);

    ///Retourner l'adresse mémoire utilisable à l'utilisateur
    return blockInsert + 1;
}
///Fonction qui incremente le compteur reference d'une case memoire, et retourne la valeur du compteur
int refinc(void *ptr){
    Case actuel = trouverCase(ptr); ///Chercher la case via son pointeur
    ///Si aucun case trouvé, retoune -1 (gestion erreur)
    if(!actuel)
        return -1;
    ///Si le compteur est 0, alors la case memoire est deja liberer, retourner -1 (gestion erreur)
    if(actuel->compt == 0){
        printf("Erreur refinc(): Pointeur deja liberee\n");
        return -1;
    }
    ///Sinon on incrémente la compteur reference et on retourne la valeur
    return ++actuel->compt;
}
///Fonction qui détruit la page libre si c'est la premiere page de la liste ou la seul page restante
void detruirepage(Case actuel){
    ///Si la page restante est libre alors on la libere avec free()
    if(actuel->taillePage == actuel->taille){
        if(nav->debut == actuel && !actuel->pageSuiv){
            //printf("Effacer la page\n");
            free(actuel);
            ///Creer un nouveau case vide
            Case caseVide = pageVideGenerateur(TAILLEPAGE);
            ///Pointe les éléments du Navigateur vers le nouveau block
            nav->debut = caseVide;       ///Premier case
            nav->fin = caseVide;        ///Dernier case
            nav->debutLibre = caseVide;   ///Premier case libre
            nav->finPage = caseVide;    ///dernier page
            return;

        }
        ///C'est la première page de la liste
        if(nav->debut == actuel){
            nav->debutLibre = actuel->libre;
            nav->debut = actuel->pageSuiv;
            free(actuel);
        }

    }
}


///Fonction qui decrement le compteur reference de la case memoire,
///si le compteur est libre alors liberer la case memoire puis fusionner si possible
void myfree(void *ptr){


    Case actuel = trouverCase(ptr);

    ///Si aucun case trouvé, le pointeur n'est pas allouer par mymalloc
    if(!actuel)
        return;

    //ptr = NULL;
    ///Si le compteur est null, alors le pointeur est deja liberee
    if(actuel->compt == 0){
        printf("Erreur myfree(): Pointeur deja liberee\n");
        return;
    }

    ///decrementer le compteur de reference
    actuel->compt--;
    ///Si le compteur est plus que zero, ne pas liberer la case
    if(actuel->compt > 0)
        return;
    ///Sinon liberer la case memoire
    if(!nav->debutLibre){
        nav->debutLibre = actuel;
    }
    else{
        void* addrActuel = (void*) actuel;
        void* addrNav = (void*) nav->debutLibre;
        if(addrActuel < addrNav){
            actuel->libre = nav->debutLibre;
            nav->debutLibre = actuel;
        }else{
            Case casePrec = actuel->prec;
            while(casePrec){
                if(casePrec->compt == 0){
                    actuel->libre = casePrec->libre;
                    casePrec->libre = actuel;
                    break;
                }
                casePrec = casePrec->prec;
            }
        }
    }
    ///Fusionner avec les prochains cases cote-a-cote libres situee dans la meme page memoire
    fusionner(actuel);
    ///Detruire la page libre si c'est la premiere page de la liste ou la seul page restante
    detruirepage(actuel);



}

int main( )
{
    int* testint1 = mymalloc(sizeof(int));
    char* testchar = mymalloc(sizeof(char));
    int* testint2 = mymalloc(sizeof(int));
    int* testint3 = testint2;
    int ref = refinc(testint3);
    printf("Nombre reference apres refinc: %d\n",ref);
    *testint1 = 10;
    printf("L'entier du testint1: %d\n", *testint1);
    imprimer();
    imprimervide();
    myfree(testint3);
    myfree(testchar);
    imprimer();
    imprimervide();
    myfree(testchar);
    int ref2 = refinc(testchar);
    printf("Nombre reference apres refinc: %d\n",ref2);
    myfree(NULL);
    void* erreur1 = mymalloc(-1);
    void* erreur2 = mymalloc(17000000);
    printf("Exemple de fusion: \n");
    myfree(testint1);
    imprimer();
    imprimervide();
    myfree(testchar);
    printf("Demander plus de 4ko memoire: \n");
    void* testgros = mymalloc(5000);
    imprimer();
    imprimervide();
    void* test4 = mymalloc(64);
    myfree(testint2);
    myfree(test4);
    printf("Detruire la page libre si c'est la premiere page de la liste ou la seul page restante\n");
    imprimer();
    imprimervide();

    return 0;
}



