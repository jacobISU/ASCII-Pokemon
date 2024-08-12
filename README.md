# ASSIGNMENT 1.10
This program was created to implement Pokemon Battles with Trainers and Wild Pokemon.
Also, Items are introduced which can be used from the PC's bag in/out of battle.
Lastly, Both PokeMarts and PokeCenters are fully functional. 

## PLAYER INVENTORY AND ITEMS
`B` - will open the player's bag revealing a random amount of Potions, Pokeballs, and Revives.(8-ish each)  
        - `1` Potion will open a window showing all player's pokemon.  
                Select the number to heal that Pokemon. Full-health wont use a potion.
        - `2` Only available during wild Pokemon battles. Otherwise, displays message.
        - `3` Revives will open a seperate window showing all player's pokemon.
                Select respective number to revive that Pokemon. Non-KO'd Pokemon wont use revive. 
        
        > NOTE:   Pressing the number of a Pokemon that doesnt exist will cause a SegFault here.

`P` - I added a Pokemon button that shows all the player's pokemon's name, HP, and Knockout status.
       
## BATTLING WILD POKEMON
The chance to encounter a pokemon in tall grass is 15%.  
The battle interface still shows a lot of the wild pokemon's stats.  

Player Interface:  
    - `1` - Fight
          Opens a move list revealing 2 moves from the player's current pokemon.
          Upon selecting one, it will activate that move.
          A Fight box will appear revealing the turn order, damage, and if any pokemon were KO'd.
          If the player's selected pokemon is KO'd, it will not attack. Must Switch if able.

    - `2` - Bag
          Same as pressing 'B' except using a pokeball will capture the Wild Pokemon.
          If player already has 6 pokemon, the wild pokemon will flee upon using the pokeball. 

    - `3` - Run
          50% chance to escape the wild pokemon battle. 
          Failing to escape forfeits your move and the wild pokemon gets a free attack.

    - `4` - Pokemon
          Opens a pokemon list. Pressing the respective number will switch out your current pokemon.
          Doing so forfeits a turn, the wild pokemon will get to attack for free.

Battle ends if the player successfully runs, the wild pokemon is caught, flees, or is knocked out.     

## BATTLING ENEMY TRAINERS
Similiar to the Wild pokemon battles except you can't run and you can't use pokeballs.
All enemy trainers can spawn with 0-2 potions/revives.
The enemy has the first turn and uses any items if needed along with selecting an move.
Then you go and use any items if needed, also selecting a move. 
Based on the priority/speed/random selection, a pokemon will go first. If the other is not KO'd, then the other will go. 
The battle only ends when either all of the Player's pokemon or the Enemy Trainer's pokemon are all KO'd.

## OTHER STUFF
- PokeMarts restock all of player's inventory items back to their base value.
- PokeCenters heal all pokemon back to full health and remove Knocked Out status.
- During a battle, pokemon will do 0 damage if they miss or a turn is forfeited. 
- Pokemon can also crit.
- If all of the Player's pokemon are KO'd, Enemy trainers will still follow but no battles will happen.

> NOTE: The windows in the fight sequence aren't perfect so an additional key press is sometimes needed.
      For 'any key' I use 5 on the numpad but any will work.  

> NOTE: Best way to experience this is to catch as many pokemon in tall grass as you can before a rival 
      or hiker gets to you. Otherwise you'll use a lot of potions just to survive the battle if they have a lot of pokemon. 

## END





 


