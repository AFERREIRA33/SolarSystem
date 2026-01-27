# System Solaire

## Objectif

Ce projet vise à créer un système solaire généré de manière procédurale. Il inclut la génération de planètes uniques, ainsi que la gestion aléatoire de leurs positions et de leur gravité au sein du système.

## Répartition des taches

 - Système solaire (logique globale et orbites) : Joseph Selva
 - Génération procédurale des planètes : Alex Ferreira

## Fonctionalités

 - Génération de planètes : Utilisation de FastNoiseLite, gestion des UV et interpolation pour créer des surfaces variées.
 - Système de gravité : Gestion dynamique de l'attraction gravitationnelle de chaque planète
 - Placement aléatoire : Génération des planètes avec un rayon, une gravité et une position initiaux aléatoires.
 - Visualisation des orbites : Utilisation de Line Trace pour représenter graphiquement la trajectoire de chaque planète.

## Variables

 - NumberOfPlanets : Nombre total de planètes à générer.
 - MinOrbitRadius / MaxOrbitRadius : Définit la zone d'apparition (distance par rapport au centre).
 - MinPlanetGravity  / MaxPlanetScale : Définit la plage de taille et de force d'attraction des planètes.
 - PlanetClass : Référence de la classe (Blueprint/Code) à instancier pour les planètes.
 - bShowwOrbits : Booléen pour afficher ou masquer le tracé des orbites.
 - OrbitResolution / OrbitLineThickness : Parametres visuels des LineTrace


## Problemes

 - Biomes : L'ajout de différents types de biomes sur les planètes est encore manquant.
 - Mapping des textures : La texture des planetes ne se génere pas par rapport à la distance avec le centre mais par rapport à la hauteur dans le monde
 - Satellites : Le système de lunes/satellites n'est pas encore fonctionnel.
 - Collisions à la génération : Les planètes peuvent apparaître trop proches les unes des autres.
 - Interférences solaires : Une trop grande proximité avec le soleil peut fausser la trajectoire orbitale calculée.



