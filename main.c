/*
 * ============================================================
 *  GESTIONNAIRE DE RESTAURANTS
 *  Projet : Programmation Avancee & Structures de Donnees
 *  Encadrant : M. Achraf Zahid
 *  EMSI Universite - Casablanca - 2eme Annee
 * 
 *  Structures de donnees utilisees :
 *    -> Liste chainee   : Gestion du menu (plats)
 *    -> File (Queue)    : Traitement des commandes (FIFO)
 *    -> Pile (Stack)    : Historique des actions (Undo/Redo)
 *    -> File prioritaire : Commandes urgentes/VIP (BONUS)
 * 
 *  Auteurs : Ayoub ABIDA, Youssef AL HAFRAWI, Ayman
 *  Date : 2025-2026
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NOM 100
#define MAX_DESC 200
#define MAX_CAT 50
#define MAX_ADRESSE 150

typedef struct Plat {
    int id;
    char nom[MAX_NOM];
    char description[MAX_DESC];
    float prix;
    char categorie[MAX_CAT];
    float note;
    struct Plat *suivant;
} Plat;

typedef struct Commande {
    int id_commande;
    char nom_client[MAX_NOM];
    int id_plat;
    char nom_plat[MAX_NOM];
    int quantite;
    float prix_total;
    int priorite;
    struct Commande *suivant;
} Commande;

typedef struct {
    Commande *tete;
    Commande *queue;
} File;

typedef struct {
    Commande *tete;
} FilePrioritaire;

typedef struct Action {
    char description[200];
    int type_action;
    Plat plat_sauvegarde;
    int position;
    int id_reference;
    struct Action *precedent;
} Action;

typedef struct {
    Action *sommet;
} Pile;

typedef struct Categorie {
    char nom[MAX_CAT];
    struct Categorie *suivant;
} Categorie;

void vider_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

Plat* creer_plat(int id, char* nom, char* desc, float prix, char* cat, float note) {
    Plat* nouveau = (Plat*)malloc(sizeof(Plat));
    if (nouveau == NULL) { printf(">> Erreur d'allocation memoire !\n"); exit(1); }
    nouveau->id = id;
    strcpy(nouveau->nom, nom);
    strcpy(nouveau->description, desc);
    nouveau->prix = prix;
    strcpy(nouveau->categorie, cat);
    nouveau->note = note;
    nouveau->suivant = NULL;
    return nouveau;
}

int generer_id(Plat* tete) {
    int max_id = 0;
    Plat* courant = tete;
    while (courant != NULL) {
        if (courant->id > max_id) max_id = courant->id;
        courant = courant->suivant;
    }
    return max_id + 1;
}

int compter_plats(Plat* tete) {
    int compteur = 0;
    Plat* courant = tete;
    while (courant != NULL) { compteur++; courant = courant->suivant; }
    return compteur;
}

Plat* rechercher_plat_par_id(Plat* tete, int id) {
    Plat* courant = tete;
    while (courant != NULL) {
        if (courant->id == id) return courant;
        courant = courant->suivant;
    }
    return NULL;
}

Plat* rechercher_plat_par_nom(Plat* tete, char* nom) {
    Plat* courant = tete;
    while (courant != NULL) {
        if (strcasecmp(courant->nom, nom) == 0) return courant;
        courant = courant->suivant;
    }
    return NULL;
}

Pile* creer_pile() {
    Pile* p = (Pile*)malloc(sizeof(Pile));
    if (p == NULL) { printf(">> Erreur d'allocation memoire !\n"); exit(1); }
    p->sommet = NULL;
    return p;
}

int pile_vide(Pile* p) { return p->sommet == NULL; }

void empiler(Pile* p, char* desc, int type, Plat* sauvegarde, int pos, int id_ref) {
    Action* nouvelle = (Action*)malloc(sizeof(Action));
    if (nouvelle == NULL) return;
    strcpy(nouvelle->description, desc);
    nouvelle->type_action = type;
    if (sauvegarde != NULL) nouvelle->plat_sauvegarde = *sauvegarde;
    nouvelle->position = pos;
    nouvelle->id_reference = id_ref;
    nouvelle->precedent = p->sommet;
    p->sommet = nouvelle;
}

Action* depiler(Pile* p) {
    if (pile_vide(p)) return NULL;
    Action* action = p->sommet;
    p->sommet = action->precedent;
    return action;
}

void ajouter_plat_debut(Plat** tete, Plat* nouveau, Pile* historique) {
    nouveau->suivant = *tete;
    *tete = nouveau;
    char desc[200];
    sprintf(desc, "Ajout debut : %s (ID:%d)", nouveau->nom, nouveau->id);
    if (historique) empiler(historique, desc, 1, NULL, 0, 0);
    printf(">> Plat \"%s\" ajoute au debut du menu avec succes !\n", nouveau->nom);
}

void ajouter_plat_fin(Plat** tete, Plat* nouveau, Pile* historique) {
    if (*tete == NULL) {
        *tete = nouveau;
    } else {
        Plat* courant = *tete;
        while (courant->suivant != NULL) courant = courant->suivant;
        courant->suivant = nouveau;
    }
    char desc[200];
    sprintf(desc, "Ajout fin : %s (ID:%d)", nouveau->nom, nouveau->id);
    if (historique) empiler(historique, desc, 1, NULL, -1, 0);
    printf(">> Plat \"%s\" ajoute a la fin du menu avec succes !\n", nouveau->nom);
}

void ajouter_plat_apres(Plat** tete, int id_reference, Plat* nouveau, Pile* historique) {
    Plat* ref = rechercher_plat_par_id(*tete, id_reference);
    if (ref == NULL) { printf(">> Plat de reference (ID:%d) introuvable !\n", id_reference); free(nouveau); return; }
    nouveau->suivant = ref->suivant;
    ref->suivant = nouveau;
    char desc[200];
    sprintf(desc, "Ajout apres ID %d : %s (ID:%d)", id_reference, nouveau->nom, nouveau->id);
    if (historique) empiler(historique, desc, 1, NULL, id_reference, id_reference);
    printf(">> Plat \"%s\" ajoute apres \"%s\" avec succes !\n", nouveau->nom, ref->nom);
}

void supprimer_plat_debut(Plat** tete, Pile* historique) {
    if (*tete == NULL) { printf(">> Le menu est vide.\n"); return; }
    Plat* a_supprimer = *tete;
    *tete = (*tete)->suivant;
    char desc[200];
    sprintf(desc, "Suppression debut : %s (ID:%d)", a_supprimer->nom, a_supprimer->id);
    if (historique) empiler(historique, desc, 2, a_supprimer, 0, 0);
    printf(">> Plat \"%s\" supprime du debut du menu.\n", a_supprimer->nom);
    free(a_supprimer);
}

void supprimer_plat_fin(Plat** tete, Pile* historique) {
    if (*tete == NULL) { printf(">> Le menu est vide.\n"); return; }
    if ((*tete)->suivant == NULL) {
        Plat* a_supprimer = *tete;
        char desc[200];
        sprintf(desc, "Suppression fin : %s (ID:%d)", a_supprimer->nom, a_supprimer->id);
        if (historique) empiler(historique, desc, 2, a_supprimer, -1, 0);
        *tete = NULL;
        printf(">> Dernier plat \"%s\" supprime.\n", a_supprimer->nom);
        free(a_supprimer);
        return;
    }
    Plat* courant = *tete;
    while (courant->suivant->suivant != NULL) courant = courant->suivant;
    Plat* a_supprimer = courant->suivant;
    char desc[200];
    sprintf(desc, "Suppression fin : %s (ID:%d)", a_supprimer->nom, a_supprimer->id);
    if (historique) empiler(historique, desc, 2, a_supprimer, -1, 0);
    courant->suivant = NULL;
    printf(">> Plat \"%s\" supprime de la fin du menu.\n", a_supprimer->nom);
    free(a_supprimer);
}

void supprimer_plat_specifique(Plat** tete, int id_reference, Pile* historique) {
    if (*tete == NULL) { printf(">> Le menu est vide.\n"); return; }
    if ((*tete)->id == id_reference) { supprimer_plat_debut(tete, historique); return; }
    Plat* courant = *tete;
    while (courant->suivant != NULL && courant->suivant->id != id_reference) courant = courant->suivant;
    if (courant->suivant == NULL) { printf(">> Plat (ID:%d) introuvable !\n", id_reference); return; }
    Plat* a_supprimer = courant->suivant;
    courant->suivant = a_supprimer->suivant;
    char desc[200];
    sprintf(desc, "Suppression specifique : %s (ID:%d)", a_supprimer->nom, a_supprimer->id);
    if (historique) empiler(historique, desc, 2, a_supprimer, 0, id_reference);
    printf(">> Plat \"%s\" (ID:%d) supprime avec succes.\n", a_supprimer->nom, a_supprimer->id);
    free(a_supprimer);
}

void modifier_plat(Plat* tete, int id, Pile* historique) {
    Plat* plat = rechercher_plat_par_id(tete, id);
    if (plat == NULL) { printf(">> Plat (ID:%d) introuvable !\n", id); return; }
    Plat sauvegarde = *plat;
    int choix;
    printf("\n--- Modification de \"%s\" ---\n", plat->nom);
    printf("1. Nom  2. Description  3. Prix  4. Categorie  5. Note  6. Tout\nChoix : ");
    scanf("%d", &choix); vider_buffer();
    switch(choix) {
        case 1: printf("Nouveau nom : "); fgets(plat->nom, MAX_NOM, stdin); plat->nom[strcspn(plat->nom, "\n")] = 0; break;
        case 2: printf("Nouvelle description : "); fgets(plat->description, MAX_DESC, stdin); plat->description[strcspn(plat->description, "\n")] = 0; break;
        case 3: printf("Nouveau prix (DH) : "); scanf("%f", &plat->prix); vider_buffer(); break;
        case 4: printf("Nouvelle categorie : "); fgets(plat->categorie, MAX_CAT, stdin); plat->categorie[strcspn(plat->categorie, "\n")] = 0; break;
        case 5: printf("Nouvelle note (0-5) : "); scanf("%f", &plat->note); if(plat->note>5)plat->note=5; if(plat->note<0)plat->note=0; vider_buffer(); break;
        default: printf("Choix invalide.\n"); return;
    }
    char desc[200];
    sprintf(desc, "Modification : %s (ID:%d)", plat->nom, plat->id);
    empiler(historique, desc, 3, &sauvegarde, 0, 0);
    printf(">> Plat modifie avec succes !\n");
}

void afficher_plats(Plat* tete) {
    if (tete == NULL) { printf("\n>> Le menu est vide.\n"); return; }
    printf("\n============================================================\n");
    printf("                   MENU DU RESTAURANT                       \n");
    printf("============================================================\n");
    printf(" ID  | Nom              | Prix DH  | Note    | Categorie    \n");
    printf("-----|------------------|----------|---------|--------------\n");
    Plat* courant = tete;
    while (courant != NULL) {
        printf(" %-3d | %-16s | %7.2f | %4.1f/5 | %-12s \n", courant->id, courant->nom, courant->prix, courant->note, courant->categorie);
        courant = courant->suivant;
    }
    printf("-----|------------------|----------|---------|--------------\n");
    printf("Total : %d plat(s)\n", compter_plats(tete));
}

void rechercher_par_categorie(Plat* tete, char* categorie) {
    if (tete == NULL) { printf("\n>> Le menu est vide.\n"); return; }
    printf("\n*** Resultats pour la categorie \"%s\" :\n", categorie);
    Plat* courant = tete; int trouve = 0;
    while (courant != NULL) {
        if (strcasecmp(courant->categorie, categorie) == 0) {
            printf("  ID:%-3d | %-20s | %7.2f DH | %.1f/5\n", courant->id, courant->nom, courant->prix, courant->note);
            trouve++;
        }
        courant = courant->suivant;
    }
    if (!trouve) printf("  Aucun plat trouve dans cette categorie.\n");
    else printf("  %d plat(s) trouve(s)\n", trouve);
}

void recherche_avancee(Plat* tete) {
    if (tete == NULL) { printf("\n>> Le menu est vide.\n"); return; }
    float prix_max, note_min; char categorie[MAX_CAT]; int filtre_cat = 0;
    printf("\nPrix maximum (0 = pas de limite) : "); scanf("%f", &prix_max); vider_buffer();
    printf("Note minimum (0-5, 0 = pas de limite) : "); scanf("%f", &note_min); vider_buffer();
    printf("Categorie (laisser vide = toutes) : "); fgets(categorie, MAX_CAT, stdin); categorie[strcspn(categorie, "\n")] = 0;
    if (strlen(categorie) > 0) filtre_cat = 1;
    printf("\n*** Resultats :\n");
    Plat* courant = tete; int trouve = 0;
    while (courant != NULL) {
        int valide = 1;
        if (prix_max > 0 && courant->prix > prix_max) valide = 0;
        if (note_min > 0 && courant->note < note_min) valide = 0;
        if (filtre_cat && strcasecmp(courant->categorie, categorie) != 0) valide = 0;
        if (valide) { printf("  ID:%-3d | %-20s | %7.2f DH | %.1f/5 | %s\n", courant->id, courant->nom, courant->prix, courant->note, courant->categorie); trouve++; }
        courant = courant->suivant;
    }
    if (!trouve) printf("  Aucun plat ne correspond aux criteres.\n");
    else printf("  %d plat(s) trouve(s)\n", trouve);
}

void echanger_plats(Plat* a, Plat* b) {
    int ti = a->id; char tn[MAX_NOM], td[MAX_DESC], tc[MAX_CAT]; float tp, tno;
    strcpy(tn, a->nom); strcpy(td, a->description); strcpy(tc, a->categorie); tp = a->prix; tno = a->note;
    a->id = b->id; strcpy(a->nom, b->nom); strcpy(a->description, b->description); strcpy(a->categorie, b->categorie); a->prix = b->prix; a->note = b->note;
    b->id = ti; strcpy(b->nom, tn); strcpy(b->description, td); strcpy(b->categorie, tc); b->prix = tp; b->note = tno;
}

void trier_plats_par_nom(Plat** tete) {
    if (*tete == NULL || (*tete)->suivant == NULL) return;
    int e; Plat *c, *d = NULL;
    do { e = 0; c = *tete; while (c->suivant != d) { if (strcasecmp(c->nom, c->suivant->nom) > 0) { echanger_plats(c, c->suivant); e = 1; } c = c->suivant; } d = c; } while (e);
    printf(">> Menu trie par nom (A->Z).\n");
}

void trier_plats_par_prix(Plat** tete) {
    if (*tete == NULL || (*tete)->suivant == NULL) return;
    int e; Plat *c, *d = NULL;
    do { e = 0; c = *tete; while (c->suivant != d) { if (c->prix > c->suivant->prix) { echanger_plats(c, c->suivant); e = 1; } c = c->suivant; } d = c; } while (e);
    printf(">> Menu trie par prix (croissant).\n");
}

void trier_plats_par_note(Plat** tete) {
    if (*tete == NULL || (*tete)->suivant == NULL) return;
    int e; Plat *c, *d = NULL;
    do { e = 0; c = *tete; while (c->suivant != d) { if (c->note < c->suivant->note) { echanger_plats(c, c->suivant); e = 1; } c = c->suivant; } d = c; } while (e);
    printf(">> Menu trie par note (meilleures d'abord).\n");
}

void trier_plats_par_categorie(Plat** tete) {
    if (*tete == NULL || (*tete)->suivant == NULL) return;
    int e; Plat *c, *d = NULL;
    do { e = 0; c = *tete; while (c->suivant != d) { if (strcasecmp(c->categorie, c->suivant->categorie) > 0) { echanger_plats(c, c->suivant); e = 1; } c = c->suivant; } d = c; } while (e);
    printf(">> Menu trie par categorie.\n");
}

int categorie_existe(Categorie* tete, char* nom) {
    Categorie* c = tete;
    while (c != NULL) { if (strcasecmp(c->nom, nom) == 0) return 1; c = c->suivant; }
    return 0;
}

Categorie* ajouter_categorie(Categorie* tete, char* nom) {
    if (categorie_existe(tete, nom)) return tete;
    Categorie* nouvelle = (Categorie*)malloc(sizeof(Categorie));
    if (nouvelle == NULL) { printf(">> Erreur d'allocation memoire !\n"); return tete; }
    strcpy(nouvelle->nom, nom); nouvelle->suivant = tete;
    printf(">> Categorie \"%s\" ajoutee avec succes !\n", nom);
    return nouvelle;
}

void afficher_categories(Categorie* tete) {
    if (tete == NULL) { printf("\n>> Aucune categorie enregistree.\n"); return; }
    printf("\n========================================\n       CATEGORIES DISPONIBLES           \n========================================\n");
    Categorie* c = tete; int i = 1;
    while (c != NULL) { printf("  %2d. %s\n", i++, c->nom); c = c->suivant; }
    printf("========================================\nTotal : %d categorie(s)\n", i-1);
}

void mettre_a_jour_categories(Categorie** tete, Plat* menu) {
    while (*tete != NULL) { Categorie* t = *tete; *tete = (*tete)->suivant; free(t); }
    Plat* c = menu;
    while (c != NULL) { *tete = ajouter_categorie(*tete, c->categorie); c = c->suivant; }
}

int supprimer_categorie(Categorie** tete, char* nom) {
    if (*tete == NULL) return 0;
    if (strcasecmp((*tete)->nom, nom) == 0) { Categorie* t = *tete; *tete = (*tete)->suivant; free(t); return 1; }
    Categorie* c = *tete;
    while (c->suivant != NULL && strcasecmp(c->suivant->nom, nom) != 0) c = c->suivant;
    if (c->suivant != NULL) { Categorie* t = c->suivant; c->suivant = t->suivant; free(t); return 1; }
    return 0;
}

void undo(Pile* p, Plat** tete) {
    if (pile_vide(p)) { printf(">> Aucune action a annuler.\n"); return; }
    Action* d = depiler(p);
    printf("\nAnnulation de : \"%s\"\n", d->description);
    switch (d->type_action) {
        case 1:
            if (d->position == 0) { if (*tete != NULL) { Plat* t = *tete; *tete = (*tete)->suivant; free(t); } }
            else if (d->position == -1) { supprimer_plat_fin(tete, NULL); }
            else if (d->id_reference > 0) { Plat* ref = rechercher_plat_par_id(*tete, d->id_reference); if (ref && ref->suivant) { Plat* t = ref->suivant; ref->suivant = t->suivant; free(t); } }
            printf("   >> Ajout annule.\n"); break;
        case 2: {
            Plat* r = creer_plat(d->plat_sauvegarde.id, d->plat_sauvegarde.nom, d->plat_sauvegarde.description, d->plat_sauvegarde.prix, d->plat_sauvegarde.categorie, d->plat_sauvegarde.note);
            if (d->position == 0) { r->suivant = *tete; *tete = r; } else { ajouter_plat_fin(tete, r, NULL); }
            printf("   >> Plat \"%s\" restaure.\n", r->nom); break;
        }
        case 3: {
            Plat* pl = rechercher_plat_par_id(*tete, d->plat_sauvegarde.id);
            if (pl) { strcpy(pl->nom, d->plat_sauvegarde.nom); strcpy(pl->description, d->plat_sauvegarde.description); pl->prix = d->plat_sauvegarde.prix; strcpy(pl->categorie, d->plat_sauvegarde.categorie); pl->note = d->plat_sauvegarde.note; printf("   >> Modifications annulees.\n"); }
            break;
        }
    }
    free(d);
}

void afficher_historique(Pile* p) {
    if (pile_vide(p)) { printf("\n>> Historique vide.\n"); return; }
    printf("\n========================================\n     HISTORIQUE DES ACTIONS (Undo)      \n========================================\n");
    Action* c = p->sommet; int i = 1;
    while (c != NULL) { printf("  %2d. %s\n", i++, c->description); c = c->precedent; }
    printf("========================================\n  %d action(s) dans l'historique\n", i-1);
}

File* creer_file() {
    File* f = (File*)malloc(sizeof(File));
    if (f == NULL) { printf(">> Erreur d'allocation memoire !\n"); exit(1); }
    f->tete = NULL; f->queue = NULL; return f;
}

int file_vide(File* f) { return f->tete == NULL; }

Commande* creer_commande(int id, char* client, int id_plat, char* nom_plat, int qte, float prix_unit, int priorite) {
    Commande* cmd = (Commande*)malloc(sizeof(Commande));
    if (cmd == NULL) { printf(">> Erreur d'allocation memoire !\n"); exit(1); }
    cmd->id_commande = id; strcpy(cmd->nom_client, client); cmd->id_plat = id_plat;
    strcpy(cmd->nom_plat, nom_plat); cmd->quantite = qte; cmd->prix_total = prix_unit * qte;
    cmd->priorite = priorite; cmd->suivant = NULL; return cmd;
}

void enfiler(File* f, Commande* cmd) {
    cmd->suivant = NULL;
    if (file_vide(f)) { f->tete = cmd; f->queue = cmd; }
    else { f->queue->suivant = cmd; f->queue = cmd; }
    printf(">> Commande #%d ajoutee a la file d'attente.\n", cmd->id_commande);
}

Commande* defiler(File* f) {
    if (file_vide(f)) { printf(">> Aucune commande en attente.\n"); return NULL; }
    Commande* cmd = f->tete; f->tete = f->tete->suivant;
    if (f->tete == NULL) f->queue = NULL;
    printf(">> Commande #%d traitee et retiree de la file.\n", cmd->id_commande);
    return cmd;
}

void afficher_file(File* f) {
    if (file_vide(f)) { printf("\n>> Aucune commande en attente.\n"); return; }
    printf("\n============================================================\n          FILE D'ATTENTE DES COMMANDES (FIFO)               \n============================================================\n");
    printf(" Cmd# | Client           | Plat             | Qte  | Total  \n------|------------------|------------------|------|--------\n");
    Commande* c = f->tete;
    while (c != NULL) { printf(" #%-4d| %-16s | %-16s | %-4d |%6.2f \n", c->id_commande, c->nom_client, c->nom_plat, c->quantite, c->prix_total); c = c->suivant; }
    printf("------|------------------|------------------|------|--------\n");
}

FilePrioritaire* creer_file_prioritaire() {
    FilePrioritaire* fp = (FilePrioritaire*)malloc(sizeof(FilePrioritaire));
    if (fp == NULL) { printf(">> Erreur d'allocation memoire !\n"); exit(1); }
    fp->tete = NULL; return fp;
}

void enfiler_prioritaire(FilePrioritaire* fp, Commande* cmd) {
    if (fp->tete == NULL) { fp->tete = cmd; cmd->suivant = NULL; }
    else if (cmd->priorite == 1) { cmd->suivant = fp->tete; fp->tete = cmd; }
    else { Commande* c = fp->tete; while (c->suivant != NULL) c = c->suivant; c->suivant = cmd; cmd->suivant = NULL; }
    if (cmd->priorite == 1) printf(">> Commande URGENTE #%d ajoutee en priorite !\n", cmd->id_commande);
    else printf(">> Commande #%d ajoutee a la file.\n", cmd->id_commande);
}

Commande* defiler_prioritaire(FilePrioritaire* fp) {
    if (fp->tete == NULL) { printf(">> Aucune commande en attente.\n"); return NULL; }
    Commande* cmd = fp->tete; fp->tete = fp->tete->suivant;
    if (cmd->priorite == 1) printf(">> Commande URGENTE #%d traitee en priorite !\n", cmd->id_commande);
    else printf(">> Commande #%d traitee.\n", cmd->id_commande);
    return cmd;
}

void afficher_file_prioritaire(FilePrioritaire* fp) {
    if (fp->tete == NULL) { printf("\n>> Aucune commande en attente.\n"); return; }
    printf("\n============================================================\n          FILE PRIORITAIRE DES COMMANDES                    \n============================================================\n");
    printf(" Cmd# | Client           | Plat             | Qte  | Priorite\n------|------------------|------------------|------|---------\n");
    Commande* c = fp->tete;
    while (c != NULL) { printf(" #%-4d| %-16s | %-16s | %-4d | %-7s\n", c->id_commande, c->nom_client, c->nom_plat, c->quantite, c->priorite == 1 ? "URGENT" : "Normal"); c = c->suivant; }
    printf("------|------------------|------------------|------|---------\n");
}

void sauvegarder_donnees(Plat* tete, Categorie* cat_tete) {
    FILE* f = fopen("sauvegarde_menu.txt", "w");
    if (f == NULL) { printf(">> Impossible de creer le fichier de sauvegarde.\n"); return; }
    fprintf(f, "=== SAUVEGARDE MENU RESTAURANT ===\n\n=== PLATS ===\n");
    Plat* c = tete;
    while (c != NULL) { fprintf(f, "ID:%d | %s | %.2f DH | %.1f | %s\n  %s\n", c->id, c->nom, c->prix, c->note, c->categorie, c->description); c = c->suivant; }
    fprintf(f, "\n=== CATEGORIES ===\n");
    Categorie* cat = cat_tete;
    while (cat != NULL) { fprintf(f, "- %s\n", cat->nom); cat = cat->suivant; }
    fclose(f);
    printf(">> Donnees sauvegardees dans 'sauvegarde_menu.txt'\n");
}

int main() {
    Plat* menu = NULL;
    Categorie* categories = NULL;
    Pile* historique = creer_pile();
    File* file_commandes = creer_file();
    FilePrioritaire* file_prio = creer_file_prioritaire();
    int compteur_commandes = 0, choix_principal;

    Plat* p1 = creer_plat(1, "Tajine Poulet", "Tajine traditionnel au poulet et legumes", 45.00, "Plats Chauds", 4.5);
    Plat* p2 = creer_plat(2, "Couscous Royal", "Couscous a la viande et aux legumes", 55.00, "Plats Chauds", 4.8);
    Plat* p3 = creer_plat(3, "Pastilla", "Pastilla aux amandes et au poulet", 40.00, "Entrees", 4.3);
    Plat* p4 = creer_plat(4, "The Menthe", "The a la menthe traditionnel", 15.00, "Boissons", 4.7);
    ajouter_plat_fin(&menu, p1, historique);
    ajouter_plat_fin(&menu, p2, historique);
    ajouter_plat_fin(&menu, p3, historique);
    ajouter_plat_fin(&menu, p4, historique);
    mettre_a_jour_categories(&categories, menu);

    printf("\n============================================================\n");
    printf("   BIENVENUE DANS LE GESTIONNAIRE DE RESTAURANTS            \n");
    printf("   Auteurs : Ayoub ABIDA, Youssef AL HAFRAWI, Ayman         \n");
    printf("   EMSI Casablanca - 2eme Annee - 2025/2026                 \n");
    printf("============================================================\n");

    do {
        printf("\n========================================\n   GESTIONNAIRE DE RESTAURANTS\n========================================\n");
        printf("  1. Gestion du menu\n  2. Gestion des commandes\n  3. Historique (Undo)\n  4. Annuler derniere action\n  5. Sauvegarder\n  0. Quitter\nChoix : ");
        scanf("%d", &choix_principal); vider_buffer();

        switch (choix_principal) {
            case 1: {
                int cg;
                do {
                    printf("\n  1. Ajouter  2. Modifier  3. Supprimer  4. Rechercher  5. Afficher  6. Categories  7. Trier  8. Retour\nChoix : ");
                    scanf("%d", &cg); vider_buffer();
                    if (cg == 1) {
                        int ca; printf("\n  1. Debut  2. Fin  3. Apres un plat  4. Retour\nChoix : "); scanf("%d", &ca); vider_buffer();
                        if (ca >= 1 && ca <= 3) {
                            char nom[MAX_NOM], desc[MAX_DESC], cat[MAX_CAT]; float prix, note;
                            printf("Nom : "); fgets(nom, MAX_NOM, stdin); nom[strcspn(nom, "\n")] = 0;
                            printf("Description : "); fgets(desc, MAX_DESC, stdin); desc[strcspn(desc, "\n")] = 0;
                            printf("Prix (DH) : "); scanf("%f", &prix); vider_buffer();
                            printf("Categorie : "); fgets(cat, MAX_CAT, stdin); cat[strcspn(cat, "\n")] = 0;
                            categories = ajouter_categorie(categories, cat);
                            printf("Note (0-5) : "); scanf("%f", &note); if(note>5)note=5; if(note<0)note=0; vider_buffer();
                            Plat* n = creer_plat(generer_id(menu), nom, desc, prix, cat, note);
                            if (ca == 1) ajouter_plat_debut(&menu, n, historique);
                            else if (ca == 2) ajouter_plat_fin(&menu, n, historique);
                            else { printf("ID reference : "); int ir; scanf("%d", &ir); vider_buffer(); ajouter_plat_apres(&menu, ir, n, historique); }
                        }
                    } else if (cg == 2) {
                        afficher_plats(menu); printf("ID a modifier : "); int id; scanf("%d", &id); vider_buffer(); modifier_plat(menu, id, historique);
                    } else if (cg == 3) {
                        int cs; printf("\n  1. Debut  2. Fin  3. Specifique  4. Retour\nChoix : "); scanf("%d", &cs); vider_buffer();
                        if (cs == 1) supprimer_plat_debut(&menu, historique);
                        else if (cs == 2) supprimer_plat_fin(&menu, historique);
                        else if (cs == 3) { printf("ID : "); int id; scanf("%d", &id); vider_buffer(); supprimer_plat_specifique(&menu, id, historique); }
                        mettre_a_jour_categories(&categories, menu);
                    } else if (cg == 4) {
                        int cr; printf("\n  1. Par nom  2. Par categorie  3. Avancee\nChoix : "); scanf("%d", &cr); vider_buffer();
                        if (cr == 1) { char nom[MAX_NOM]; printf("Nom : "); fgets(nom, MAX_NOM, stdin); nom[strcspn(nom, "\n")] = 0; Plat* t = rechercher_plat_par_nom(menu, nom); if (t) printf("\nTrouve : %s | %.2f DH | %.1f/5\n", t->nom, t->prix, t->note); else printf(">> Introuvable.\n"); }
                        else if (cr == 2) { char cat[MAX_CAT]; printf("Categorie : "); fgets(cat, MAX_CAT, stdin); cat[strcspn(cat, "\n")] = 0; rechercher_par_categorie(menu, cat); }
                        else if (cr == 3) recherche_avancee(menu);
                    } else if (cg == 5) afficher_plats(menu);
                    else if (cg == 6) {
                        int cc; printf("\n  1. Afficher  2. Ajouter  3. Supprimer  4. Retour\nChoix : "); scanf("%d", &cc); vider_buffer();
                        if (cc == 1) afficher_categories(categories);
                        else if (cc == 2) { char nom[MAX_CAT]; printf("Nom : "); fgets(nom, MAX_CAT, stdin); nom[strcspn(nom, "\n")] = 0; categories = ajouter_categorie(categories, nom); }
                        else if (cc == 3) { char nom[MAX_CAT]; printf("Nom : "); fgets(nom, MAX_CAT, stdin); nom[strcspn(nom, "\n")] = 0; if (supprimer_categorie(&categories, nom)) printf(">> Supprimee.\n"); else printf(">> Introuvable.\n"); }
                    } else if (cg == 7) {
                        int ct; printf("\n  1. Nom  2. Prix  3. Note  4. Categorie  5. Retour\nChoix : "); scanf("%d", &ct); vider_buffer();
                        if (ct == 1) { trier_plats_par_nom(&menu); afficher_plats(menu); }
                        else if (ct == 2) { trier_plats_par_prix(&menu); afficher_plats(menu); }
                        else if (ct == 3) { trier_plats_par_note(&menu); afficher_plats(menu); }
                        else if (ct == 4) { trier_plats_par_categorie(&menu); afficher_plats(menu); }
                    }
                } while (cg != 8);
                break;
            }
            case 2: {
                int cc;
                do {
                    printf("\n  1. Commande normale  2. Commande URGENTE  3. Traiter  4. Afficher  5. Retour\nChoix : ");
                    scanf("%d", &cc); vider_buffer();
                    if (cc == 1 || cc == 2) {
                        if (menu == NULL) { printf(">> Menu vide.\n"); break; }
                        afficher_plats(menu);
                        printf("ID plat : "); int ip; scanf("%d", &ip); vider_buffer();
                        Plat* plat = rechercher_plat_par_id(menu, ip);
                        if (!plat) { printf(">> Introuvable.\n"); break; }
                        char client[MAX_NOM]; int qte;
                        printf("Nom client : "); fgets(client, MAX_NOM, stdin); client[strcspn(client, "\n")] = 0;
                        printf("Quantite : "); scanf("%d", &qte); vider_buffer();
                        compteur_commandes++;
                        Commande* cmd = creer_commande(compteur_commandes, client, plat->id, plat->nom, qte, plat->prix, cc == 2 ? 1 : 0);
                        enfiler(file_commandes, cmd);
                        Commande* cmd2 = creer_commande(compteur_commandes, client, plat->id, plat->nom, qte, plat->prix, cc == 2 ? 1 : 0);
                        enfiler_prioritaire(file_prio, cmd2);
                    } else if (cc == 3) {
                        int cf; printf("  1. File normale  2. File prioritaire\nChoix : "); scanf("%d", &cf); vider_buffer();
                        if (cf == 1) { Commande* t = defiler(file_commandes); if (t) free(t); }
                        else if (cf == 2) { Commande* t = defiler_prioritaire(file_prio); if (t) free(t); }
                    } else if (cc == 4) { afficher_file(file_commandes); afficher_file_prioritaire(file_prio); }
                } while (cc != 5);
                break;
            }
            case 3: afficher_historique(historique); break;
            case 4: undo(historique, &menu); mettre_a_jour_categories(&categories, menu); break;
            case 5: sauvegarder_donnees(menu, categories); break;
            case 0: printf("\nMerci ! EMSI Casablanca - 2025/2026\n"); break;
            default: printf(">> Choix invalide.\n");
        }
    } while (choix_principal != 0);

    while (menu != NULL) { Plat* t = menu; menu = menu->suivant; free(t); }
    while (categories != NULL) { Categorie* t = categories; categories = categories->suivant; free(t); }
    while (!pile_vide(historique)) { Action* t = depiler(historique); free(t); } free(historique);
    while (!file_vide(file_commandes)) { Commande* t = defiler(file_commandes); free(t); } free(file_commandes);
    while (file_prio->tete != NULL) { Commande* t = defiler_prioritaire(file_prio); free(t); } free(file_prio);

    printf("Memoire liberee. Programme termine.\n\n");
    return 0;
}
