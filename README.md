# Valve-Secure-Server
```sh-session
PPV-Publish-Feature (Valve Anticheat uid_ban) - Premier Players VAC can only be used by admin (license limited)
```


## Installation:
<details>
<summary>Project (Drop Down)</summary>
  
### [0] : Installations Project:Source
  * Extract the package and upload it to your server
  * can choose to lock the location  ( Head / Body / foot )
  * Use `meta list` to check if the plugin works
  * Add to the `csgo/addons/configs/admins.ini`  file your steamid in any format
  * Used Console `status` for see the player `uid_steam`
  * Used Console `mm_ban <userid> <time in minutes>` For ban players main
  * Used Console `mm_unban <steamid>` for unban players

  
### [1] : License
      * Updated valve_slack
  </details>


**gameinfo.gi IS RESET AFTER EVERY UPDATE**

> [!NOTE]  
> admins.ini - This file editable only on stop server or not loading plugin
```
"Admins"
{
    "[U:1:1137445611]"  "1/0" // any value, this don't checks, don't add steamid if doesn't trust him
}
```
> [!NOTE]  
> bans.ini - This file editable only on stop server or not loading plugin
```
"BanList"
{
    "[U:1:1137445611]"  "0" // integer value in minutes, 0 = permanent, this file editable only on stop server or not loading plugin, in another ways use commands mm_ban/mm_unban
}
```
![image](https://i.ytimg.com/vi/QR9XwGwaJAU/maxresdefault.jpg)






<h2 align="center"> Copyright © 2023 - @imhunterand
