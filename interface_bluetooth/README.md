# Bluettoth

#### Installation

```
sudo apt-get update && sudo apt-get install bluez libbluetooth-dev
```

#### Liaison Bluetooth

Pour la communication bluetooth, vous devrez configurer deux ordinateurs (Par exemple un Pi et un portable). L'un d'entre eux sera le *serveur* (celui qui envoie le signal) et l'autre sera le *client* (celui qui reçoit le signal). La procédure de configuration est la suivante :

Les étapes suivantes sont à exécuter **sur les deux ordinateurs**:

1. Lancez le *daemon* de gestion Bluetooth avec la commande `sudo systemctl start bluetooth`; notez que vous pouvez lancer automatiquement le daemon à chaque démarrage, en exécutant `sudo systemctl enable bluetooth`.
2. Assurez-vous que le périphérique Bluetooth est prêt à recevoir/émettre : `sudo hciconfig hci0 up`
3. À l'aide de la commande `hciconfig`, récupérer l'adresse MAC de l'interface Bluetooth de votre serveur, vous en aurez besoin dans le code du client pour amorcer la communication avec le serveur.

#### Transmission via Bluetooth

La communication se fait à l'aide d'un socket tel que vous avez implanté dans le laboratoire 2. Veuillez vous référer à l'exemple sur ce [répertoire](https://github.com/balle/bluetooth-snippets) pour vous aider. Dans le cadre de ce laboratoire, vous pouvez utiliser un lien rfcomm qui garantit la transmission des données dans leurs intégralités et dans l'ordre d'envoi (similaire au protocole TCP). Cependant, si vous essayez d'envoyer directement l'audio, vous constaterez deux problèmes gênants :

1. Le débit du lien série est beaucoup trop faible. Obtenir au-delà de 30 ko/s est déjà un tour de force et nécessite une distance très faible entre les deux Pi. Dans ces conditions, il est impensable d'envoyer un signal sonore non modifié, qui nécessite au moins 100 ko/s de bande passante...
2. Les latences de réception des paquets sont elles aussi très variables. Cela est dû à la structure du Bluetooth lui-même, qui attend qu'un paquet soit demandé avant de l'envoyer effectivement. Aussi, si bien souvent la latence est compatible avec un traitement sonore, des pauses de plusieurs centaines de millisecondes peuvent se produire relativement régulièrement, ce qui est un problème pour l'audio.

En ce qui concerne le premier problème, nous vous suggèrons d'*encoder* l'audio dans un format *compressé* afin de limiter sa bande passante à une valeur compatible avec la vitesse du lien Bluetooth. Voir [l'exemple suivant](todo).

Pour ce qui est du second problème, nous allons utiliser une stratégie s'appuyant sur le fait qu'*en moyenne*, la latence est correcte : ce sont les pics de latence qui doivent être éliminés. Nous allons utiliser un tampon pour limiter ces pics de latence. Du côté serveur, rien à changer : ce dernier envoie encore aussi rapidement que possible les paquets contenant l'audio compressé. Cependant, du côté client, un *thread* supplémentaire devra être créé : ce *thread* ne fera que lire les données le plus vite possible et les stocker dans un tampon. En moyenne, ce tampon sera rempli plus vite qu'il ne sera vidé, puisque la vitesse moyenne du Bluetooth est supérieure à la vitesse à laquelle les données sont lues. Lorsqu'une longue pause se produit, ce tampon sera graduellement vidé, évitant ainsi au processus envoyant le son dans l'interface audio de manquer de données et de produire de disgracieux craquements.