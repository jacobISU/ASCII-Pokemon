# ASSIGNMENT 1.10

This program was created to implement Pokémon battles with Trainers and Wild Pokémon. It also introduces items that can be used from the PC's bag in and out of battle. Additionally, both Poké Marts and Poké Centers are fully functional.

## PLAYER INVENTORY AND ITEMS

- **`B`**: Opens the player's bag, revealing a random amount of Potions, Poké Balls, and Revives (approximately 8 each).
  - **`1`**: Potion will open a window showing all of the player's Pokémon. Select the number to heal that Pokémon. Full-health Pokémon won’t use a Potion.
  - **`2`**: Available only during Wild Pokémon battles. Otherwise, it displays a message.
  - **`3`**: Revives will open a separate window showing all of the player's Pokémon. Select the number to revive that Pokémon. Non-KO'd Pokémon won’t use a Revive.

  > **NOTE**: Pressing the number of a Pokémon that doesn’t exist will cause a segmentation fault.

- **`P`**: Shows all the player's Pokémon names, HP, and Knockout status.

## BATTLING WILD POKEMON

The chance to encounter a Pokémon in tall grass is 15%. The battle interface displays many of the Wild Pokémon’s stats.

**Player Interface:**
- **`1` - Fight**: Opens a move list revealing 2 moves from the player's current Pokémon. Upon selecting one, it will activate that move. A Fight box will appear showing the turn order, damage, and any KO'd Pokémon. If the player's selected Pokémon is KO'd, it will not attack. Must switch if able.
- **`2` - Bag**: Functions like pressing `B`, but using a Poké Ball will attempt to capture the Wild Pokémon. If the player already has 6 Pokémon, the Wild Pokémon will flee when a Poké Ball is used.
- **`3` - Run**: 50% chance to escape the Wild Pokémon battle. Failing to escape forfeits your move, and the Wild Pokémon gets a free attack.
- **`4` - Pokémon**: Opens a Pokémon list. Pressing the respective number will switch out your current Pokémon, forfeiting a turn and allowing the Wild Pokémon a free attack.

**Battle Ends**: If the player successfully runs, the Wild Pokémon is caught, flees, or is knocked out.

## BATTLING ENEMY TRAINERS

Similar to Wild Pokémon battles, but you can't run or use Poké Balls. Enemy Trainers can have 0-2 Potions/Revives. The enemy goes first, using items if needed and selecting a move. The player then uses any items if needed and selects a move. Based on priority, speed, and random selection, Pokémon will take turns. The battle ends when all of the Player’s or the Enemy Trainer’s Pokémon are KO'd.

## OTHER STUFF

- Poké Marts restock all of the player's inventory items back to their base values.
- Poké Centers heal all Pokémon to full health and remove KO status.
- During a battle, Pokémon deal 0 damage if they miss or if a turn is forfeited. Critical hits are possible.
- If all of the Player’s Pokémon are KO'd, Enemy Trainers will still follow, but no battles will occur.

> **NOTE**: The windows in the fight sequence aren’t perfect, so an additional key press may be needed. For 'any key', use `5` on the numpad, but any key will work.

> **NOTE**: To get the best experience, catch as many Pokémon as possible in tall grass before encountering a rival or hiker. Otherwise, you may use many Potions to survive battles if they have numerous Pokémon.

## END
