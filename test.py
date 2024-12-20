from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
import json
import time

def get_type_and_move_from_cell(cell):
    try:
        retreat_cost_images = []
        move_name = ""
        type_images = []
        after_line = False

        elements = cell.find_elements(By.XPATH, "./*")
        
        for element in elements:
            if element.tag_name == "hr" and "a-table__line" in element.get_attribute("class"):
                after_line = True
                continue

            if not after_line:
                if element.tag_name == "img":
                    src = element.get_attribute("data-src")
                    retreat_cost_images.append(src)
            else:
                if element.tag_name == "img":
                    alt_text = element.get_attribute("alt")
                    src = element.get_attribute("data-src")
                    type_images.append((alt_text, src))
                elif element.tag_name == "b":
                    move_name = element.text.strip()

        text_content = cell.text
        damage = ""
        if after_line:
            damage = text_content.split()[-1]

        return retreat_cost_images, move_name, type_images, damage
    except Exception as e:
        print(f"Error extracting from cell: {e}")
        return [], "", [], ""

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
        "move_name": "",
        "move_type": [],
        "damage": ""
    }

    try:
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
        
        retreat_cost, move_name, type_images, damage = get_type_and_move_from_cell(columns[9])
        card_data["retreat_cost"] = retreat_cost
        card_data["move_name"] = move_name
        card_data["move_type"] = type_images
        card_data["damage"] = damage

    except Exception as e:
        print(f"Error processing column: {e}")

    return card_data

# Set up Chrome options
chrome_options = Options()
chrome_options.add_argument("--headless")  # Run in headless mode (no GUI)

# Initialize the Chrome driver
driver = webdriver.Chrome(options=chrome_options)

# URL of the page to scrape
url = "https://game8.co/games/Pokemon-TCG-Pocket/archives/482685"

# Navigate to the URL
driver.get(url)

# Wait for the table to load (using the correct class)
wait = WebDriverWait(driver, 10)
table = wait.until(EC.presence_of_element_located((By.CSS_SELECTOR, "table.a-table.a-table.table--fixed.flexible-cell.a-table")))

# Find the tbody element
tbody = table.find_element(By.TAG_NAME, "tbody")

# Find all rows within the tbody
rows = tbody.find_elements(By.TAG_NAME, "tr")

# Initialize a list to store card data
card_data = []

# Process each row
for row in rows:
    try:
        card = parse_card_row(row)
        card_data.append(card)
    except Exception as e:
        print(f"Error processing row: {e}")

# Close the browser
driver.quit()

# Write data to JSON file
with open('pokemon_tcg_pocket_cards.json', 'w', encoding='utf-8') as f:
    json.dump(card_data, f, ensure_ascii=False, indent=4)

print(f"Number of cards extracted: {len(card_data)}")
print("Card data has been written to pokemon_tcg_pocket_cards.json")
