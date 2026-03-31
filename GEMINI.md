# 🌿 Plantasia - Carnet de Bord Architecture (GEMINI.md)

**Description :** Application interactive de simulation de balcon et encyclopédie botanique/pédologique.  
**Stack Technique :** C++17, SFML 2.6.1, ImGui (docking branch), nlohmann/json.

---

## 🎯 Objectif Actuel : Refactoring Architectural & UI "Pro"
Nous sommes en train de migrer d'une architecture monolithique (où la donnée, la logique et l'UI étaient mélangées dans les `States`) vers une architecture modulaire MVC/Domain-Driven.

### 🗺️ Plan de Refactoring (En cours)
- [x] **Étape 1 :** Externaliser les données en JSON (Plantes, Sols, Pots, Boutures).
- [x] **Étape 2 :** Créer `DataLoader` / `DatabaseManager` (Parser le JSON vers les Structs C++).
- [x] **Étape 3 :** Créer un bouclier anti-null pour le parseur JSON.
- [x] **Étape 4 :** Interconnecter les données (Boutons dynamiques Plante <-> Sol <-> Pot <-> Bouture).
- [x] **Étape 5 :** Ajouter des Tooltips interactifs pour expliquer le lexique technique (CEC, Drainage, etc.).
- [ ] **Étape 6 :** Extraire les Helpers UI (`ExplicationDrainage`, `CouleurRusticite`, etc.) dans `Utils/EnumInfo.hpp`.
- [ ] **Étape 7 :** Créer `Utils/ColorTheme.hpp` pour centraliser la palette de couleurs.
- [ ] **Étape 8 :** Découper le monolithe `StateWiki.cpp` en sous-fonctions (`DrawLeftPanel()`, `DrawPlantDetails()`, etc.).
- [ ] **Étape 9 :** Créer des Widgets UI réutilisables (`UI::NavButton`, `UI::TagBadge`, `UI::InfoCard`).
- [ ] **Étape 10 :** Optimisation : Mettre en cache le tri des listes dans le `DatabaseManager`.

---

## 📝 Historique des Sessions

### Session X (Dernière session) - *Refonte DB & Stabilisation UI*
* **Data :** Finalisation des 4 structures de données (`Plante`, `SolData`, `PotData`, `BoutureData`) et de leurs macros `NLOHMANN_JSON_SERIALIZE_ENUM`.
* **Fix Critique :** Résolution des crashs `ImGui::End()` (Stack assertion) causés par des `BeginChild()` mal équilibrés lors du changement d'onglets.
* **Fix Critique :** Résolution des erreurs de parsing JSON (`type must be string, but is null`) avec un helper lambda sécurisé `get_safe_string()`.
* **UI/UX :** Implémentation d'un système de Tooltips au survol `(?)` pour les caractéristiques techniques des sols et boutures, évitant de surcharger l'interface.
* **Feature :** Rendu fonctionnel du `StateCatalogue` avec drag & drop des plantes et shader de contour (Outline Shader).

---

## 🏗️ Architecture Cible (Rappel)

* **Core :** `Application`, `StateMachine`, `State` -> *Gère la boucle principale.*
* **Data :** `*.json` -> *Aucune logique, que de la donnée.*
* **Repository :** `DatabaseManager` -> *Charge les JSON, gère la mémoire, instancie les Structs.*
* **Domain :** Structs C++ pures -> *0 dépendance à ImGui ou SFML.*
* **UI / States :** `StateWiki`, `StateCatalogue` -> *Gère uniquement l'affichage et les inputs.*
* **Utils :** `EnumInfo`, `ColorTheme` -> *Helpers de rendu.*

---

## 🐛 Bugs Connus / Dette Technique Actuelle
* Le tri des plantes dans `StateWiki` et `StateCatalogue` se fait à chaque frame dans la boucle `drawImGui()`. À déplacer dans le `DatabaseManager` (initialisation unique).
* `StateWiki.cpp` est trop long (+700 lignes) car il contient toute la logique de rendu et les dictionnaires d'énums.

---

## 🚀 Prochaine Étape (À copier pour lancer la prochaine session)
**"Salut Gemini ! On reprend le projet Plantasia. Voici mon fichier `GEMINI.md` à jour. J'aimerais qu'on s'attaque à l'Étape 6 et 7 du plan : Extraire les Helpers UI dans un fichier `EnumInfo.hpp` et créer le `ColorTheme.hpp`. Voici mon `StateWiki.cpp` actuel pour commencer le découpage :"**