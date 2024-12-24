from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
import json
import re

# Function to extract moves and abilities from text content
def extract_moves_and_abilities(text_content):
    # Split the text content by lines
    lines = text_content.split('\n')
    
    moves = []
    abilities = []
    
    for line in lines:
        # Check if the line contains a move with damage
        match = re.match(r"(.*)\s+(\d+)$", line)
        if match:
            move_name = match.group(1).strip()
            damage = match.group(2).strip()
            moves.append({'move_name': move_name, 'damage': damage})
        else:
            # If no damage is found, consider it as an ability or description
            ability = line.strip()
            if ability:
                abilities.append(ability)
    
    return moves, abilities

# Function to parse a single row of the table
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
        "abilities": []
    }

    try:
        # Extract basic card data
        card_data["#"] = columns[1].text.strip()
        card_data["card_name"] = columns[2].find_element(By.TAG_NAME, "a").text.strip()
        
        rarity_img = columns[3].find_element(By.XPATH, ".//img")
        card_data["rarity"] = rarity_img.get_attribute("alt").split(" - ")[-1] if rarity_img else ""
        
        card_data["exclusive_pack"] = columns[4].text.strip()
        
        type_img = columns[5].find_element(By.XPATH, ".//img")
        card_data["type"] = type_img.get_attribute("alt") if type_img else ""
        card_data["type_image_url"] = type_img.get_attribute("data-src") if type_img else ""
        
        card_data["hp"] = columns[6].text.strip()
        card_data["stage"] = columns[7].text.strip()
        card_data["pack_points"] = columns[8].text.strip()
        
        # Extract retreat cost and effect details from the 9th column
        retreat_cost_images, text_content = [], ""
        
        # Process retreat cost images (before the horizontal line)
        retreat_cost_elements = columns[9].find_elements(By.XPATH, ".//img")
        for img in retreat_cost_elements:
            src = img.get_attribute("data-src")
            retreat_cost_images.append(src)
        
        # Extract text content for moves and abilities (after the horizontal line)
        text_content = columns[9].text
        
        # Parse moves and abilities from text content
        moves, abilities = extract_moves_and_abilities(text_content)
        # print(moves, abilities, retreat_cost_images)
        # Update card data with parsed information
        card_data["retreat_cost"] = retreat_cost_images
        card_data["moves"] = moves
        card_data["abilities"] = abilities

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
