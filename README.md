#KONSPEKT
Gra w kółko i krzyżyk w trybie wieloosobowym
Jan Kuliga, Krzysztof Bera, Konrad Sikora

ZAŁOŻENIA:
Gra w kółko i krzyżyk umożliwiająca jednoczesną rozgrywkę wielu osobom korzystającym z urządzeń będących w jednej sieci lokalnej (LAN).

STRUKTURA:
Program serwera współbieżnego będzie uruchomiony na jednej z maszyn, ma odpowiadać m.in. za zestawianie połączeń między dwoma graczami. 
 Gracze będą się łączyć z serwerem za pośrednictwem programu klienta, natomiast sama rozgrywka będzie się odbywać trybie Peer to Peer, tj. pomiędzy dwoma maszynami graczy. Gracz będzie mógł m.in. zaakceptować lub odrzucić zaproszenie do gry przez innego gracza.

PROTOKÓŁ WARSTWY TRANSPORTOWEJ:
TCP 

ADRESACJA SIECIOWA:
IPv4

STYL KODU:
https://www.kernel.org/doc/html/v4.10/process/coding-style.html
