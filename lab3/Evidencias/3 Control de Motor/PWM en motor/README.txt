En este directorio se encuentra el código utilizado para determinar el valor mínimo de PWM
con el cual el motor comienza a girar suavemente, evitando movimientos bruscos o disconti-
nuos (“trancasos”).

El objetivo de esta prueba fue identificar el umbral de arranque del motor, de modo que en
el control  final se utilice  ese valor como límite inferior del PWM (≈120), asegurando un
movimiento progresivo y estable.