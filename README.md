# alphabet-war
baseed on socket in linux. service and client communicate by socket. The player operates the alphabet 'M' to play the game. The rebot is alphabet 'W'. Player and rebot can move and fire . The aim for player is destroy all robots.player can create or join room for playing together .
client using curses ploting.

# installation curses
<pre><code>sudo apt-get install libncurses5-dev
</code></pre>

# Compile
<pre><code>
gcc s.c -lpthread           // for service
gcc c.c -lpthread -lcurses //  for client
</code></pre>

# Screenshot
 ![image](https://github.com/ldcodes/alphabet-war/blob/master/1.png)


