import json

# Load the JSON file
with open('pokemon_tcg_pocket_cards.json', 'r') as file:
    data = json.load(file)

# Add 'weakness' field to each item in the list
for item in data:
    energy_type = item['type']
    weakness_type = ''
    if energy_type == 'Grass': weakness_type = 'Fire'
    elif energy_type == 'Fire': weakness_type = 'Water'
    elif energy_type == 'Water': weakness_type = 'Lightning'
    elif energy_type == 'Lightning': weakness_type = 'Fighting'
    elif energy_type == 'Psychic': weakness_type = 'Darkness'
    elif energy_type == 'Fighting': weakness_type = 'Psychic'
    elif energy_type == 'Darkness': weakness_type = 'Fighting'
    elif energy_type == 'Metal': weakness_type = 'Fire'
    elif energy_type == 'Dragon': weakness_type = ''
    elif energy_type == 'Colorless': weakness_type = 'Fighting'
    
    card_name = item['card_name'].lower()
    
    if card_name in ['moltres', 'zapdos', 'pidgey', 'pidgeotto', 'pidgeot', 'spearow', 'fearow', "farfetch'd", 'doduo', 'dodrio', 'aerodactyl', 'aerodactyl ex', 'pidgeot ex', 'chatot']:
        item['weakness'] = 'Lightning'
    elif card_name in ['snom', 'frosmoth', 'clefairy', 'clefable', 'flabebe', 'floette', 'florges', 'swirlix', 'slurpuff']:
        item['weakness'] = 'Metal'
    elif card_name in ['mankey', 'primeape', 'machop', 'machoke', 'machamp', 'machamp ex', 'hitmonlee', 'hitmonchan', 'miefoo', 'mienshao', 'clobbopus', 'grapploct', 'marshadow']:
        item['weakness'] = 'Psychic'
    elif card_name in ['purrloin', 'liepard']:
        item['weakness'] = 'Grass'
    item['weakness'] = weakness_type

# Save the updated data back to the JSON file
with open('simulator/pokemon_tcg_pocket_cards.json', 'w+') as file:
    json.dump(data, file, indent=4)
