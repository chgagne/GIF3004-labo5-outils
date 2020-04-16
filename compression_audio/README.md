# Compression [Opus](https://en.wikipedia.org/wiki/Opus_(audio_format))

## Installation

```
sudo apt-get update && sudo apt-get install libopus-dev
```

Également, n'oubliez pas de transférer les fichiers d'en-tête et les librairies de *libopus* dans le *sysroot* de votre station de travail. Référez-vous au [laboratoire 1](http://vision.gel.ulaval.ca/~cgagne/enseignement/SETR/H2020/labo1.html) pour la procédure de synchronisation entre votre environnement de compilation croisée et votre Raspberry Pi.

## Exemple d'utilisation

Des exemples d'utilisation et de paramétrisation de la librairie Opus sont présenté dans les fichiers *encodeur_opus.c* et *decodeur_opus.c* afin de vous aider à compresser et décompresser vos signals audio.

## Échantillonnage
Notez que le taux d'échantillonnage standard des CD est de 44.1Khz, par contre Opus ne supporte pas ce taux. Il est donc nécessaire de rééchantillonner le signal audio (ex. 48Khz) avant de le compresser. 

Vous pouvez utiliser un outil comme [SoX](http://sox.sourceforge.net/) pour convertir votre fichier:
```
sox input.wav -r 48000 output.wav rate
```
