# Alsa

#### Lecture du son sur le port HDMI

Puisque le Raspberry Pi Zero ne possède pas de sortie audio, nous pouvons utiliser le port HDMI. Il sera donc nécessaire de décommenter la ligne `hdmi_drive=2` dans le fichier */boot/config.txt* du Pi qui va envoyer le son aux haut-parleurs. Assurez-vous de redémarrer ce dernier avant de continuer.

#### Acquisition du son à partir de Alsa

Afin de lire le son à partir de l'entrée USB, nous allons utiliser la librairie Alsa, qui constitue le standard *de facto* de la gestion audio sous Linux. Le fichier *audio.c*, disponible dans le dépôt de code, fournit un exemple d'utilisation d'Alsa dans ce contexte. Notez que si vous n'avez pas une carte d'acquisition sous la main, vous pouvez *simuler* une entrée microphone ou ligne en utilisant [le module *aloop* de Alsa](https://www.alsa-project.org/main/index.php/Matrix:Module-aloop). Ce module permet de connecter une source sonore (par exemple un fichier WAV que vous aurez préalablement téléchargé sur le Pi) comme s'il s'agissait d'un périphérique réel. Pour l'utiliser, chargez d'abord le module correspondant : `sudo modprobe snd-aloop`. Par la suite, vous pourrez observer qu'un nouveau périphérique est apparu dans la liste fournie par `aplay -l`. Repérez les identifiants *device* et *subdevice* de ce pseudo-périphérique. Lancez par la suite le programme *aplay* sur ce périphérique :

```
aplay -D hw:DEV,SUBDEV monfichier.wav
```

avec, bien évidemment, *DEV* et *SUBDEV* remplacés par le numéro de votre périphérique. Une fois cela fait, vous pouvez lancer votre programme d'acquisition, en lui fournissant le même périphérique de *loopback*. De cette manière, vous pouvez tester votre code en toutes circonstances.

> Nous vous fournissons deux fichiers WAV ([le premier](http://wcours.gel.ulaval.ca/2019/h/GIF3004/default/8fichiers/labo5/habanera.wav) et [le second](http://wcours.gel.ulaval.ca/2019/h/GIF3004/default/8fichiers/labo5/toreador.wav)) d'une trentaine de secondes chacun d'une guitare électrique « claire » (sans effets appliqués). Notez que ces fichiers sont stéréo. Ces extraits proviennent de [karoryfesamples](https://soundcloud.com/karoryfersamples) et sont disponibles sous licence d'utilisation non commerciale. Vous pouvez évidemment utiliser vos propres fichiers.
