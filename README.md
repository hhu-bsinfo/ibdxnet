# other very useful resources

http://www.rdmamojo.com/2013/02/02/ibv_post_recv/

# ib commands

ibstat -> list available infiniband devices and state
ibstatus -> show ib device status
ibnodes -> show available ib nodes in network
iblinkinfo -> show switch layout with connected nodes, identifiers and total uplink

# ibperf

# before all commands on all machines, to align cpu freq to performance mode, run:
for CPUFREQ in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do [ -f $CPUFREQ ] || continue; echo -n performance > $CPUFREQ; done

## latency rdma write test
## replace ib_write_lat with ib_read_lat for read latency test
#server:
ib_write_lat -a -F
#client:
ib_write_lat -a -F node51

## rdma write bandwidth test
## replace ib_write_bw with ib_read_bw for write bandwidth test
#server
ib_write_bw -a -F
#client:
ib_write_bw -a -F node51


# setup

# Konfugration von Infiniband Adaptern

#### Wichtige Information
- dieser Guide bezieht sich nur auf Infiniband Karten von [Mellanox](http://www.mellanox.com/index.php)


## Installation 
Laden Sie sich den für ihr Adapter geeigneeten Treiber von der [Mellanox Homepage](http://www.mellanox.com/page/software_overview_ib) runter. Hierbei ist es wichtig zu beachten, dass der Treiber ausgewählt wird, der wirklich für das Betriebssystem geeignet ist! (In diesem Guide wird die Konfiguration in einer Ubuntu 16.04 x64_86 Umgebung durchgeführt).

Danach muss der heruntergeladene Treiber installiert werden. Hierzu muss, die im entpackten Treiber vorhandene, `mlnxofedinstall` Datei ausgeführt werden (falls die libvma Bibliothek im späteren Verlauf verwenden werden soll, empfiehlt es sich das --vma Flag mit anzugeben). Während der Installation kann es die Fehlergebung geben, dass die `libfind` Bibliothek nicht vorhanden ist. In diesem Fall kann diese mit dem Kommando `sudo apt-get install libfind-lib-perl` nachinstalliert werden.

Nachdem die Installation abgeschlossen ist, kann überprüft werden, ob diese erfolgreich war. Führen Sie hierzu das Kommando `sudo hca_self_test.ofed` aus. Wenn die Ausgabe des Kommandos ähnlich wie die folgende Abbildung aussieht, kann mit der Konfiguration fortgesetzt werden (sonst müssen die obigen Schritte wiederholt werden).
![alt text](./doc/hca_self_test.png)

## Konfiguration

Damit der Adapter als Netzwerk-Schnittstelle anerkannt wird, müssen folgende Zeilen in die `/etc/network/interfaces` Datei eingefügt werden. (In diesem Beispiel bekommt der Adapter die IP-Adresse 10.0.1.51)
```
auto ib0
iface ib0 inet static
	address 10.0.1.51
	netmask 255.255.255.0
```
Anschließend muss der Treiber neu geladen werden. Hierzu kann folgendes Kommando verwendet werden - `sudo /etc/init.d/openibd restart`. Falls nach dem Neuladen des Treiber beim Aufruf des `ifconfig` Kommandos der Adapter angezeigt wird, kann die Konfiguration fortgesetzt werden. Zusätzlich hierzu sollte jedoch auch das `ibstat -p` Kommando verwendet werden um zu überprüfen, ob dem Adapter eine GUID zugewissen wurde.

Abschließend muss der Subnetmanager `opensm` gestartet werden. Hierzu muss das `sudo systemctl start opensm` Kommando eingegeben werden. Anschließend sollte kontrolliert werden, ob der Start des Subnetmanagers erfolgreich durchgeführt wurde. Hierzu kann folgendes Kommando hilfreich sein - `systemctl status opensm`.

## Testen

Wenn mehrere Adapter zur Verfügung stehen, kann mithilfe der `ibhosts` Funktion überprüft werden, ob diese sich untereinander finden. Ein Beispiel hierfür ist nachfolgend dargestellt.
![alt text](./doc/ibhosts.png)

Damit das Anpingen klappt, muss ein Adapter im Servermode das "ibping" Tool starten. Hierzu wird das Kommando `ibping -S` verwendet. Um diesen Adapter anzupingen wird folgednes Kommando für verwendet - `ibping -G <GUID>`. (Um die GUID des Servers rauszufinden, kann vor dem Starten des ibping-Tools das ibstat Tool verwendet werden)