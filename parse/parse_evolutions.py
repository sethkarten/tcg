import json
import requests
from bs4 import BeautifulSoup

def load_json(filename):
    with open(filename, 'r') as f:
        return json.load(f)

def save_json(data, filename):
    with open(filename, 'w') as f:
        json.dump(data, f, indent=2)

# List of baby Pokémon
baby_pokemon = [
    "Pichu", "Cleffa", "Igglybuff", "Togepi", "Tyrogue", "Smoochum", "Elekid", "Magby", "Azurill", "Wynaut", "Budew", "Chingling", "Bonsly", "Mime Jr.", "Happiny", "Munchlax", "Riolu", "Mantyke", "Toxel"
]

def get_evolution_data():
    url = "https://pokemondb.net/evolution"
    response = requests.get(url)
    soup = BeautifulSoup(response.content, 'html.parser')
    
    evolution_data = {}
    all_pokemon = set()
    
    for pokemon in soup.find_all('a', class_='ent-name'):
        all_pokemon.add(pokemon.text.strip())
    
    for element in soup.find_all(['h2', 'div']):
        if element.name == 'h2':
            continue
        
        if 'infocard-list-evo' in element.get('class', []):
            current_chain = []
            for pokemon in element.find_all('div', class_='infocard'):
                name = pokemon.find('a', class_='ent-name').text.strip()
                if name not in baby_pokemon:
                    current_chain.append(name)
            
            for i, name in enumerate(current_chain):
                if i == 0:
                    evolution_data[name] = ""  # Base form
                else:
                    evolution_data[name] = current_chain[i - 1]

     # Add fossil Pokémon
    fossil_data = {
        "Omanyte": "Helix Fossil",
        "Kabuto": "Dome Fossil",
        "Aerodactyl": "Old Amber",
        "Lileep": "Root Fossil",
        "Anorith": "Claw Fossil",
        "Cranidos": "Skull Fossil",
        "Shieldon": "Armor Fossil",
        "Tirtouga": "Cover Fossil",
        "Archen": "Plume Fossil",
        "Tyrunt": "Jaw Fossil",
        "Amaura": "Sail Fossil"
    }

    for pokemon, fossil in fossil_data.items():
        evolution_data[pokemon] = fossil
    
    for pokemon in all_pokemon:
        if pokemon not in evolution_data and pokemon not in baby_pokemon:
            evolution_data[pokemon] = ""
    
    return evolution_data

def process_tcg_cards(tcg_cards, evolution_data):
    for card in tcg_cards:
        card_name = card['card_name']
        
        if card_name.endswith(" ex"):
            card_name = card_name[:-3]
        
        if card_name in evolution_data:
            card['evolves_from'] = evolution_data[card_name]
        else:
            card['evolves_from'] = ""
    
    return tcg_cards

if __name__ == "__main__":
    evolution_data = get_evolution_data()
    tcg_cards = load_json("pokemon_tcg_pocket_cards.json")
    updated_tcg_cards = process_tcg_cards(tcg_cards, evolution_data)
    save_json(updated_tcg_cards, "pokemon_tcg_pocket_cards.json")
    print("Updated TCG cards data saved to pokemon_tcg_pocket_cards_with_evolution.json")
