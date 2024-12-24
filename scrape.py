from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
import json
import re

def extract_moves_and_abilities(text_content, card_name):
    lines = text_content.strip().split('\n')
    moves = []
    abilities = []
    retreat_cost = []
    current_move = None
    current_ability = None

    if card_name == 'Butterfree':
        print(lines)

    i = 0

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
            current_move = {"name": line.strip(), "energy": [], "damage": "", "description": ""}
        elif re.search(r"(Grass|Fire|Water|Lightning|Psychic|Fighting|Dark|Colorless)(\s+\d+)?", line):
            if current_move:
                current_move["energy"].extend([f"{e}{n}" for e, n in re.findall(r"(Grass|Fire|Water|Lightning|Psychic|Fighting|Dark|Colorless)(\s+\d+)?", line)])
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

    return moves, abilities, retreat_cost


def parse_card_row(row):
    columns = row.find_elements(By.TAG_NAME, "td")
    
    card_data = {
        "#": "",
        "card_name": "",
        "rarity": "",
        "exclusive_pack": "",
        "type": "",
        "type_image_url": "",
        "hp": "",
        "stage": "",
        "pack_points": "",
        "retreat_cost": [],
        "moves": [],
        "ability": "None"
    }

    try:
        card_data["#"] = columns[1].text.strip()
        card_data["card_name"] = columns[2].find_element(By.TAG_NAME, "a").text.strip()
        
        rarity_img = columns[3].find_element(By.XPATH, ".//img")
        card_data["rarity"] = rarity_img.get_attribute("alt").split(" - ")[-1] if rarity_img else ""
        
        card_data["exclusive_pack"] = columns[4].text.strip()
        
        type_img = columns[5].find_element(By.XPATH, ".//img")
        card_data["type"] = type_img.get_attribute("alt").split(' - ')[-1] if type_img else ""
        card_data["type_image_url"] = type_img.get_attribute("data-src") if type_img else ""
        
        card_data["hp"] = columns[6].text.strip()
        card_data["stage"] = columns[7].text.strip()
        card_data["pack_points"] = columns[8].text.strip()
        
        text_content = columns[9].text
        
        moves, abilities, retreat_cost = extract_moves_and_abilities(text_content, card_data["card_name"])
        
        card_data["retreat_cost"] = retreat_cost
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
