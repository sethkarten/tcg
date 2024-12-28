from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
import json
import re

def substring_in_list(string_list, substring):
    """
    Checks if a given substring exists in any string within a list.
    
    Args:
        string_list: A list of strings.
        substring: The substring to search for.
    
    Returns:
        True if the substring is found in any string within the list, False otherwise.
    """
    for string in string_list:
        if substring in string:
            return True
    return False

# n/a
# https://img.game8.co/3998614/b92af68265b2e7623de5efdf8197a9bf.png/show

def extract_moves_and_abilities(text_content, energy_list):
    lines = text_content.strip().split('\n')
    moves = []
    abilities = []
    retreat_cost = []
    current_move = None
    current_ability = None

    i = 0
    move_counter = 0

    while i < len(lines):
        line = lines[i]
        if line.startswith("Retreat Cost:"):
            retreat_cost = [""]
        elif line.startswith("[Ability]"):
            if current_ability:
                abilities.append(current_ability)
            ability_name = line.replace("[Ability]", "").strip()
            current_ability = {"name": ability_name, "description": ""}
            current_ability["description"] += lines[i+1].strip() + " "
            i += 1
        elif re.match(r"^[A-Za-z\s]+$", line):  # Move name
            if current_move:
                moves.append(current_move)
            energy_str = ''
            if len(energy_list) > 0:
                energy_str =  energy_list[move_counter]
            current_move = {"name": line.strip(), "energy": energy_str, "damage": "", "description": ""}
            move_counter += 1
        elif line.isdigit() or line[:-1].isdigit():
            if current_move:
                current_move["damage"] = line.strip()
        elif current_move and not line.startswith("["):
            current_move["description"] += line.strip() + " "
        i += 1

    if current_move:
        moves.append(current_move)
    if current_ability:
        abilities.append(current_ability)

    return moves, abilities

def process_energy_string(energy_string):
    match = re.match(r"(\D+?)\s*(\d+)?$", energy_string)
    if match:
        energy = match.group(1).strip()
        number_part = match.group(2)
        num_energies = int(number_part) if number_part else 1
    else:
        energy = energy_string.strip()
        num_energies = 1
    
    return num_energies, energy


def parse_card_row(row):
    columns = row.find_elements(By.TAG_NAME, "td")
    
    card_data = {
        "#": "",
        "card_name": "",
        "rarity": "",
        "exclusive_pack": "",
        "type": "",
        # "type_image_url": "",
        "hp": "",
        "stage": "",
        "pack_points": "",
        "retreat_cost": [],
        "moves": [],
        "ability": "None"
    }
    
    card_retreat_dict = {
       "https://img.game8.co/3994730/6e5546e2fbbc5a029ac79acf2b2b8042.png/show" : "1",
       "https://img.game8.co/3998538/eea8469456d6b7ea7a2daf2995087d00.png/show": "2",
       "https://img.game8.co/3998539/6bb558f97aac02e469e3ddc06e2ac167.png/show": "3",
       "https://img.game8.co/3998556/3831ed9a23dbc9db0da4254334165863.png/show": "4",
       "https://img.game8.co/3998614/b92af68265b2e7623de5efdf8197a9bf.png/show": "0",
    }

    try:
        card_data["#"] = columns[1].text.strip()
        card_data["card_name"] = columns[2].find_element(By.TAG_NAME, "a").text.strip()
        
        rarity_img = columns[3].find_element(By.XPATH, ".//img")
        card_data["rarity"] = rarity_img.get_attribute("alt").split(" - ")[-1] if rarity_img else ""
        
        card_data["exclusive_pack"] = columns[4].text.strip()
        
        type_img = columns[5].find_element(By.XPATH, ".//img")
        card_data["type"] = type_img.get_attribute("alt").split(' - ')[-1] if type_img else ""
        # card_data["type_image_url"] = type_img.get_attribute("data-src") if type_img else ""
        
        card_data["hp"] = columns[6].text.strip()
        card_data["stage"] = columns[7].text.strip()
        card_data["pack_points"] = columns[8].text.strip()
        
        text_content = columns[9].text
        
        try:
            energy_list = []
            energy_images = columns[9].find_elements(By.XPATH, ".//img[contains(@class, 'a-img')]")
            for img in energy_images[1:]:
                alt = img.get_attribute("alt")
                # print(alt)
                energy_list.append(alt)
            energy_list_per_move = []
            move_energies = []
            for energy in energy_list:
                if substring_in_list(move_energies, energy.split(' ')[0]) or ('Mew ex' in card_data["card_name"] and len(move_energies) != 0):
                    energy_list_per_move.append(move_energies)
                    move_energies = []
                num_energies, energy = process_energy_string(energy)
                for i in range(num_energies):
                    move_energies.append(energy)
            energy_list_per_move.append(move_energies)
            energy_list = energy_list_per_move
        except:
            energy_list = []
        
        if 'Mew ex' in card_data["card_name"]:
            print(text_content)
            print(energy_list)
        moves, abilities = extract_moves_and_abilities(text_content, energy_list)
        try:
            retreat_cost_img = columns[9].find_element(By.XPATH, ".//img")
            card_data["retreat_cost"] = retreat_cost_img.get_attribute("data-src") if type_img else ""
            if card_data["retreat_cost"] != "":
                card_data["retreat_cost"] = card_retreat_dict[card_data["retreat_cost"]]
        except:
            card_data["retreat_cost"] = ""
        card_data["moves"] = moves
        card_data["ability"] = abilities

    except Exception as e:
        print(f"Error processing row: {e}")

    return card_data

# Set up Chrome options for Selenium
chrome_options = Options()
chrome_options.add_argument("--headless")  # Run in headless mode (no GUI)

# Initialize Chrome WebDriver
driver = webdriver.Chrome(options=chrome_options)

# URL of the page to scrape
url = "https://game8.co/games/Pokemon-TCG-Pocket/archives/482685"

# Navigate to the URL
driver.get(url)

# Wait for the table to load (using the correct class name for the table)
wait = WebDriverWait(driver, 10)
table = wait.until(EC.presence_of_element_located((By.CSS_SELECTOR, "table.a-table.a-table.table--fixed.flexible-cell.a-table")))

# Find the tbody element and all rows within it
tbody = table.find_element(By.TAG_NAME, "tbody")
rows = tbody.find_elements(By.TAG_NAME, "tr")

# Initialize a list to store all parsed card data
card_data_list = []

# Process each row in the table
for row in rows:
    try:
        card_data_list.append(parse_card_row(row))
    except Exception as e:
        print(f"Error processing row: {e}")

# Close the browser after scraping is complete
driver.quit()

# Write parsed data to a JSON file
with open('pokemon_tcg_pocket_cards.json', 'w', encoding='utf-8') as f:
    json.dump(card_data_list, f, ensure_ascii=False, indent=4)

print(f"Number of cards extracted: {len(card_data_list)}")
print("Card data has been written to pokemon_tcg_pocket_cards.json")
