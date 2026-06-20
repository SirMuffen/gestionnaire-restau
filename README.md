# 🍽️ Gestionnaire de Restaurants

Projet réalisé dans le cadre du module **Programmation Avancée & Structures de Données**  
Encadrant : M. Achraf Zahid | EMSI Casablanca — 2ème Année IIR | 2025-2026  
Auteurs : **Ayoub ABIDA**, Youssef AL HAFRAWI, Ayman

---

## 📌 Description

Application console en **Langage C** de gestion complète d'un restaurant, 
implémentant 4 structures de données fondamentales.

---

## 🧱 Structures de données utilisées

| Structure | Utilisation |
|-----------|-------------|
| Liste Chaînée | Gestion du menu (ajout début/fin/après, suppression, tri) |
| File (Queue) FIFO | Traitement des commandes dans l'ordre d'arrivée |
| Pile (Stack) | Système Undo/Redo — annulation des dernières actions |
| File Prioritaire | Commandes urgentes/VIP traitées en priorité |

---

## ⚙️ Fonctionnalités

- ✅ Ajout / Suppression / Modification de plats
- ✅ Tri multi-critères : par nom, prix, note, catégorie
- ✅ Recherche avancée (prix max, note min, catégorie)
- ✅ Système Undo/Redo complet (pile d'historique)
- ✅ Gestion des commandes FIFO + prioritaires
- ✅ Sauvegarde des données dans un fichier `.txt`
- ✅ Libération dynamique de la mémoire (free)

---

## 🚀 Compilation & Exécution

```bash
gcc main.c -o restaurant
./restaurant