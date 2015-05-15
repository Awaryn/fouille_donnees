#include "scindeur.h"

// calcule la taille du vocabulaire et le nombre de documents par catégorie
// requiert que nb_docs ait ses élements initialisés à 0

int question1(uint16_t* nb_docs) {

    int i;
    for (i = 0; i < NB_CAT; i++) {
        nb_docs[i] = 0; // on intialise tous les éléments à 0
    }

    FILE* fichier = fopen("BaseReuters-29", "r");
    int num_mot, nb_occur;
    int fin_de_ligne = 0;
    int fin_de_fichier = 0;
    int fscanf_value;
    int max = 0;
    int num_cat;

    fscanf(fichier, "%d", &num_cat);
    while (!fin_de_fichier) {
        fin_de_ligne = 0;
        nb_docs[num_cat - 1]++;
        //printf("La categorie est : %d\n", num_cat);
        while (!fin_de_ligne) {
            fscanf_value = fscanf(fichier, " %d:%d", &num_mot, &nb_occur);
            if (num_mot > max) max = num_mot;
            if (fscanf_value == 2) {
                //printf("Le mot %d est present %d fois dans ce document\n", num_mot, nb_occur);
            } else if (fscanf_value == -1) {
                fin_de_ligne = 1;
                fin_de_fichier = 1;
            } else { // on a lu la categorie de la prochaine ligne
                fin_de_ligne = 1;
                num_cat = num_mot;
            }
        }
    }
    fclose(fichier);

    return max;
}

void question2(List **base_apprentissage, List **base_test) {
    List *base_app = *base_apprentissage;
    List *base_t = *base_test;

    time_t t;
    double r;

    /* Intializes random number generator */
    srand((unsigned) time(&t));

    // ratio idealement voulu
    const double ratio = (double) TAILLE_APPRENTISSAGE_VOULUE / (double) NB_TOTAL_DOCUMENTS;
    /* on scinde aleatoire la collection suivant deux ensembles ; 
    base d'entraînement (environ 52500 documents) et base de test (environ 18203 documents). 
     */
    int i;
    for (i = NB_TOTAL_DOCUMENTS; i > 0; i--) { // on part de la fin car on ajoute en debut de chaine
        r = r2();
        if (r < ratio) {
            base_app = cons(i, base_app);
        } else {
            base_t = cons(i, base_t);
        }
    }

    *base_apprentissage = base_app;
    *base_test = base_t;
}

// Algorithm 5: Modèle multivarié de Barnoulli, phase d'apprentissage

void apprentissageBernoulli(List *base_app, uint16_t N[NB_CAT],
        uint16_t** df) {

    // on initialise les chaines de df avec un element de debut et de fin
    // on initialise également les N[k] à 0
    init(N, df);

    FILE* fichier = fopen("BaseReuters-29", "r");
    int num_cat, num_mot, nb_occur;
    uint16_t** dfk;
    int fin_de_ligne = 0;
    int fin_de_fichier = 0;
    int est_app; //fait partie ou non de la base d'apprentissage
    int fscanf_value;
    uint32_t num_ligne = 1;

    fscanf(fichier, "%d", &num_cat);
    while (!fin_de_fichier && (base_app != NULL)) {
        fin_de_ligne = 0;
        dfk = &(df[num_cat - 1]);
        if (num_ligne == base_app->val) {
            est_app = 1;
            List *cell = base_app;
            base_app = base_app->next; // on avance dans la base d'apprentissage
            free(cell);
            N[num_cat - 1]++;
        } else {
            est_app = 0;
        }

        while (!fin_de_ligne) {
            fscanf_value = fscanf(fichier, " %d:%d", &num_mot, &nb_occur);
            if (fscanf_value == 2) {
                if (est_app) {
                    ((*dfk)[num_mot - 1])++;
                }
            } else if (fscanf_value == -1) {
                fin_de_ligne = 1;
                fin_de_fichier = 1;
            } else { // on a lu la categorie de la prochaine ligne
                fin_de_ligne = 1;
                num_ligne++;
                num_cat = num_mot;
            }
        }
    }

    fclose(fichier);

    assert(base_app == NULL);



}

// Algorithm 6: Modèle multivarié de Barnoulli, phase de test

int testBernoulli(List *base_test, uint16_t N[NB_CAT],
        uint16_t** df, int m) {

    // on initialise les PiF[k]
    double PiF[NB_CAT];
    initPiF(PiF, N, df, m);

    FILE* fichier = fopen("BaseReuters-29", "r");
    int num_cat, num_mot, nb_occur;
    int num_cat_prec;
    int fin_de_ligne = 0;
    int fin_de_fichier = 0;
    int est_test; //fait partie ou non de la base de test
    int fscanf_value;
    uint32_t num_ligne = 1;
    int nbJuste = 0;

    uint16_t** dfk;

    fscanf(fichier, "%d", &num_cat);
    while (!fin_de_fichier && (base_test != NULL)) {
        //printf("Ligne %d\n", num_ligne); //DEBUG
        fin_de_ligne = 0;
        if (num_ligne == base_test->val) {
            est_test = 1;
            List *cell = base_test;
            base_test = base_test->next; // on avance dans la base de test
            free(cell);
        } else {
            est_test = 0;
        }

        // on construit la chaine des mots de la ligne
        List *motsLigne = NULL;
        while (!fin_de_ligne) {
            fscanf_value = fscanf(fichier, " %d:%d", &num_mot, &nb_occur);
            if (fscanf_value == 2) {
                if (est_test) {
                    motsLigne = cons(num_mot, motsLigne);
                }
            } else if (fscanf_value == -1) {
                fin_de_ligne = 1;
                fin_de_fichier = 1;
                num_cat_prec = num_cat;
            } else { // on a lu la categorie de la prochaine ligne
                fin_de_ligne = 1;
                num_ligne++;
                num_cat_prec = num_cat;
                num_cat = num_mot;
            }
        }
        if (est_test) { // si ce doc fait partie de la base de test
            // on la met dans l'ordre
            motsLigne = inverser(motsLigne);

            // on estime la classe de ce document (cette ligne)
            double max = -DBL_MAX;
            uint8_t k_max = 0;
            uint8_t k;
            for (k = 0; k < NB_CAT; k++) {
                List *mot = motsLigne;
                double PiFk = PiF[k];
                dfk = &(df[k]);
                uint16_t Nk = N[k];
                while (mot != NULL) {
                    // wid = 1 ; il faut mettre à jour avec la bonne valeur
                    PiFk += log((double) ((*dfk)[mot->val - 1] + 1)) - log((double) (Nk - (*dfk)[mot->val - 1] + 1));
                    mot = mot->next;
                }
                // on met à jour le max et son indice
                if (max < PiFk) {
                    max = PiFk;
                    k_max = k;
                }
            }

            // on regarde si cette estimation est bonne
            if (k_max == (num_cat_prec - 1)) {
                nbJuste++;
            }

            // on libere la memoire
            free_list(motsLigne);
        }
    }

    fclose(fichier);

    assert(base_test == NULL);

    return nbJuste;
}

// Algorithm 7: Modèle multinomial, phase d'apprentissage

void apprentissageMultinomial(List *base_app, uint16_t N[NB_CAT], uint32_t D[NB_CAT],
        uint16_t** tf) {

    // on initialise les chaines de df avec un element de debut et de fin
    // on initialise également les N[k] à 0
    init2(N, D, tf);

    FILE* fichier = fopen("BaseReuters-29", "r");
    int num_cat, num_mot, nb_occur;
    uint16_t** tfk;
    int fin_de_ligne = 0;
    int fin_de_fichier = 0;
    int est_app; //fait partie ou non de la base d'apprentissage
    int fscanf_value;
    uint32_t num_ligne = 1;

    fscanf(fichier, "%d", &num_cat);
    while (!fin_de_fichier && (base_app != NULL)) {
        fin_de_ligne = 0;
        tfk = &(tf[num_cat - 1]);
        if (num_ligne == base_app->val) {
            est_app = 1;
            List *cell = base_app;
            base_app = base_app->next; // on avance dans la base d'apprentissage
            free(cell);
            N[num_cat - 1]++;
        } else {
            est_app = 0;
        }

        while (!fin_de_ligne) {
            fscanf_value = fscanf(fichier, " %d:%d", &num_mot, &nb_occur);
            if (fscanf_value == 2) {
                if (est_app) {
                    (*tfk)[num_mot - 1] += nb_occur;
                    D[num_cat - 1] += nb_occur;
                }
            } else if (fscanf_value == -1) {
                fin_de_ligne = 1;
                fin_de_fichier = 1;
            } else { // on a lu la categorie de la prochaine ligne
                fin_de_ligne = 1;
                num_ligne++;
                num_cat = num_mot;
            }
        }
    }

    fclose(fichier);
    assert(base_app == NULL);
}

// Algorithm 8: Modèle multinomial, phase de test

int testMultinomial(List *base_test, uint16_t N[NB_CAT], uint32_t D[NB_CAT],
        uint16_t** tf, int m) {

    // on initialise les PiF[k]
    double PiF[NB_CAT];
    int k;
    for (k = 0; k < NB_CAT; k++) {
        PiF[k] = log((double) N[k] / (double) m);
    }

    FILE* fichier = fopen("BaseReuters-29", "r");
    int num_cat, num_mot, nb_occur;
    int num_cat_prec;
    int fin_de_ligne = 0;
    int fin_de_fichier = 0;
    int est_test; //fait partie ou non de la base de test
    int fscanf_value;
    uint32_t num_ligne = 1;
    int nbJuste = 0;

    uint16_t** tfk;

    fscanf(fichier, "%d", &num_cat);
    while (!fin_de_fichier && (base_test != NULL)) {
        //printf("Ligne %d\n", num_ligne); //DEBUG
        fin_de_ligne = 0;
        if (num_ligne == base_test->val) {
            est_test = 1;
            List *cell = base_test;
            base_test = base_test->next; // on avance dans la base de test
            free(cell);
        } else {
            est_test = 0;
        }

        // on construit la chaine des mots de la ligne
        List *motsLigne = NULL;
        while (!fin_de_ligne) {
            fscanf_value = fscanf(fichier, " %d:%d", &num_mot, &nb_occur);
            if (fscanf_value == 2) {
                if (est_test) {
                    motsLigne = cons(num_mot, motsLigne);
                }
            } else if (fscanf_value == -1) {
                fin_de_ligne = 1;
                fin_de_fichier = 1;
                num_cat_prec = num_cat;
            } else { // on a lu la categorie de la prochaine ligne
                fin_de_ligne = 1;
                num_ligne++;
                num_cat_prec = num_cat;
                num_cat = num_mot;
            }
        }
        if (est_test) { // si ce doc fait partie de la base de test
            // on la met dans l'ordre
            motsLigne = inverser(motsLigne);

            // on estime la classe de ce document (cette ligne)
            double max = -DBL_MAX;
            uint8_t k_max = 0;
            uint8_t k;
            for (k = 0; k < NB_CAT; k++) {
                List *mot = motsLigne;
                double PiFk = PiF[k];
                tfk = &(tf[k]);
                uint32_t Dk = D[k];
                while (mot != NULL) {
                    // wid = 1 ; il faut mettre à jour avec la bonne valeur
                    PiFk += log((double) ((*tfk)[mot->val - 1] + 1)) - log((double) (Dk + taille_voca));
                    mot = mot->next;
                }
                // on met à jour le max et son indice
                if (max < PiFk) {
                    max = PiFk;
                    k_max = k;
                }
            }

            // on regarde si cette estimation est bonne
            if (k_max == (num_cat_prec - 1)) {
                nbJuste++;
            }

            // on libere la memoire
            free_list(motsLigne);
        }
    }

    fclose(fichier);
    assert(base_test == NULL);

    return nbJuste;
}

int main() {

    /* Question 1 */

    printf("\nQuestion 1 :\n");
    uint16_t nb_docs[NB_CAT];
    taille_voca = question1(nb_docs);

    printf("La taille du vocabulaire est : %d\n", taille_voca);
    int somme_verif = 0;
    int i;
    for (i = 0; i < NB_CAT; i++) {
        printf("La classe %d contient %d documents\n", i + 1, nb_docs[i]);
        somme_verif += nb_docs[i];
    }
    assert(somme_verif == NB_TOTAL_DOCUMENTS);



    uint16_t** df = malloc(sizeof(uint16_t*)*NB_CAT);
    for (i = 0; i < NB_CAT; i++) {
      df[i] = malloc(sizeof(uint16_t)*taille_voca);
    }

    int nb;
    for (nb = 0; nb < 1; nb++) {

        /* Question 2 */

        printf("\nQuestion 2 :\n");
        List *base_apprentissage = NULL;
        List *base_test = NULL;
        question2(&base_apprentissage, &base_test);

        printf("Base d'apprentissage :\n");
        int m_app = print(base_apprentissage);
        printf("Base de test :\n");
        int m_test = print(base_test);


        /* Question 3 */

        printf("\nQuestion 3 :\n");
        uint16_t N[NB_CAT];
        //printf("DECLARATION OK\n");

        apprentissageBernoulli(base_apprentissage, N, df);


        /* Question 4 */

        printf("\nQuestion 4 :\n");
        int nbJuste = testBernoulli(base_test, N, df, m_app);
        printf("Dans la base de test du modèle multivarié de Bernoulli,\n");
        printf("Le taux de bonne classification est de %f\n", (double) (100 * nbJuste) / (double) m_test);

    }

    uint16_t** tf = malloc(sizeof(uint16_t*)*NB_CAT);
    for (i = 0; i < NB_CAT; i++) {
      tf[i] = malloc(sizeof(uint16_t)*taille_voca);
    }

    for (nb = 0; nb < 1; nb++) {

        /* Question 2 */
        printf("\nQuestion 2 :\n");
        List *base_apprentissage = NULL;
        List *base_test = NULL;
        question2(&base_apprentissage, &base_test);

        printf("Base d'apprentissage :\n");
        int m_app = print(base_apprentissage);
        printf("Base de test :\n");
        int m_test = print(base_test);


        /* Question 3 */

        printf("\nQuestion 3 :\n");
        uint16_t N[NB_CAT];
        uint32_t D[NB_CAT];
        //printf("DECLARATION OK\n");

        apprentissageMultinomial(base_apprentissage, N, D, tf);


        /* Question 4 */

        printf("\nQuestion 4 :\n");
        int nbJuste = testMultinomial(base_test, N, D, tf, m_app);
        printf("Dans la base de test du modèle multinomial,\n");
        printf("Le taux de bonne classification est de %f\n", (double) (100 * nbJuste) / (double) m_test);

    }

    for (i = 0; i < NB_CAT; i++) {
      free(df[i]);
      free(tf[i]);
    }
    free(df);
    free(tf);


    return (0);
}
