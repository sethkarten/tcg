from selenium import webdriver
from selenium.webdriver.common.by import By
from selenium.webdriver.chrome.options import Options
import json
import time
import re

# Set up Chrome options
chrome_options = Options()
chrome_options.add_argument("--headless")  # Run in headless mode (no GUI)

# Initialize Chrome driver
driver = webdriver.Chrome(options=chrome_options)

# URL of the page to scrape
url = "https://game8.co/games/Pokemon-TCG-Pocket/archives/482685"

# Navigate to the URL
driver.get(url)

# Allow time for content to load
time.sleep(5)  # Adjust sleep time as necessary

# Scroll down to load more content if needed (optional)
driver.execute_script("window.scrollTo(0, document.body.scrollHeight);")
time.sleep(3)  # Wait for additional content to load

# Get all card elements (adjust selector based on actual structure)
cards = driver.find_elements(By.XPATH, "//table[contains(@class, 'a-table')]//tr")

# Image URL mapping for types
image_url_mapping = {
    "https://img.game8.co/3994730/6e5546e2fbbc5a029ac79acf2b2b8042.png/show": "colorless",
    "https://img.game8.co/4018726/c2d96eaebb6cd06d6a53dfd48da5341c.png/show": "grass",
    "https://img.game8.co/4018725/13914d1a973822da2863205cffe8d814.png/show": "fire",
    "https://img.game8.co/3998542/1a14eac8ad4df81b977a2a4f2c412715.png/show": "water",
    "https://img.game8.co/4018727/9851d3f597a114b1ab6ef669071cda7c.png/show": "lightning",
    "https://img.game8.co/4018729/5d54ec566203717af2c7b7a14f69e0d7.png/show": "psychic",
    "https://img.game8.co/3998533/3d670ee685179925d9480e4430606d55.png/show": "fighting",
    "https://img.game8.co/3998536/889ef61097e8b0df4f4fda2f5b7f63fd.png/show": "dark"
}



# Function to extract type from an image element
def get_type_from_image(cell):
    try:
        img = cell.find_element(By.TAG_NAME, "img")
        alt_text = img.get_attribute("alt")  # Get the alt text
        src = img.get_attribute("data-src")  # Get the src image link
        type_name = alt_text.split(" - ")[-1] if alt_text else ""  # Extract type name from alt text
        return type_name.lower(), alt_text, src  # Return type (in lowercase) and image URL
    except:
        return "", "", ""  # Return empty if no image is found



def get_type_and_move_from_cell(cell):
    try:
        retreat_cost_images = []
        move_name = ""
        type_images = []
        after_line = False

        # Get all child elements
        elements = cell.find_elements(By.XPATH, "./*")
        
        for element in elements:
            if element.tag_name == "span" and "a-table__line" in element.get_attribute("class"):
                after_line = True
                continue

            if not after_line:
                if element.tag_name == "img":
                    src = element.get_attribute("data-src")
                    retreat_cost_images.append(src)
            else:
                if element.tag_name == "img":
                    alt_text = element.get_attribute("alt")
                    type_name = alt_text.split(" - ")[-1] if alt_text else ""
                    type_images.append(type_name.lower())
                else:
                    move_name += element.text.strip() + " "

        # Get text content
        text_content = cell.text
        if after_line:
            move_name += text_content.strip() + " "

        move_name = move_name.strip()
        return retreat_cost_images, move_name, type_images
    except Exception as e:
        print(f"Error extracting from cell: {e}")
        return [], "", []




# Function to correct the card data format
def correct_card_data(card):
    # Extract type and type image URL from the type cell
    card_type, _, src_type = get_type_from_image(card["type_cell"])
    retreat_cost_images, move_name, type_images = get_type_and_move_from_cell(card["effect"])
    print(retreat_cost_images)
    print(move_name)
    print(type_images)
    input()
    
    corrected_card = {
        "#": card["card_number"],
        "card_name": card["card_name"],
        "rarity": card["rarity"],
        "exclusive_pack": card["exclusive_pack"],
        "type": card_type,
        "hp": card["hp"],
        "stage": card["stage"],
        "pack_points": card["pack_points"],
        "retreat_cost": card["retreat_cost"],
        "effect": "",
        "ability": ""
    }
    return corrected_card

# Initialize a list to store card data
card_data = []

# Iterate through each card row (skip header)
for row in cards[1:]:
    columns = row.find_elements(By.TAG_NAME, "td")
    if len(columns) >= 10:  # Ensure there are enough columns
        # Parse raw data from columns
        raw_card_data = {
            "card_number": columns[1].text.strip(),
            "card_name": columns[2].text.strip(),
            "rarity": columns[3].text.strip(),
            "exclusive_pack": columns[4].text.strip(),
            "type_cell": columns[5],  # Pass the entire cell for type extraction
            "hp": columns[6].text.strip(),
            "stage": columns[7].text.strip(),
            "pack_points": columns[8].text.strip(),
            "retreat_cost": columns[9].text.strip(),
            "effect": columns[9]
        }
        
        # Correct and format the data
        # corrected_card = raw_card_data
        corrected_card = correct_card_data(raw_card_data)
        card_data.append(corrected_card)

# Close the browser
driver.quit()

# Write data to JSON file
with open('pokemon_tcg_pocket_cards.json', 'w', encoding='utf-8') as f:
    json.dump(card_data, f, ensure_ascii=False, indent=4)

print(f"Number of cards extracted: {len(card_data)}")
print("Card data has been written to pokemon_tcg_pocket_cards.json")
