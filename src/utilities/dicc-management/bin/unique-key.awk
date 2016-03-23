#! /usr/bin/awk -f

# aquest script unifica en una sola linia,
# totes les linies que tinguin la mateixa 
# clau.
#
# p.e. si l'entrada es
#
#   K1 a b c
#   K2 e f 
#   K2 e h g
#   K3 a
#   K4 b
#   K4 c d
#
# la sortida sera
#
#   K1 a b c
#   K2 e f e h g
#   K3 a
#   K4 b c d
#
# ATENCIO: Per a que funcioni l'entrada ha d'estar
# ordenada (o sigui, totes les linies de la mateixa
# clau han d'estar juntes)
#
# Es pot cridar fent:
#   cat myfile | sort | clau-unica >resultat
#
# Si el fitxer te un separador no-blanc (p.e. '#')
# cal fer:
#   cat myfile | sort | clau-unica -F'#' >resultat
#

# ---------------------------------
# obtenir primera linia i inicialitzar
BEGIN {getline; key=$1; data=substr($0,length($1)+2)}

{ if (key==$1) {  
    # si la clau es igual que la linia anterior, acumular camps
    data=data FS substr($0,length($1)+2)
  }
  else { 
    # si es diferent, escriure l'acumulat i reinicialitzar
    print key FS data;
    key=$1; data=substr($0,length($1)+2);
  }
}

# escriure l'ultima clau
END { print key FS data; }

