from selenium import webdriver
from selenium.webdriver.common.keys import Keys
from selenium.common.exceptions import TimeoutException
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC
from selenium.webdriver.common.by import By
from selenium.webdriver.common.action_chains import ActionChains
import time
import csv
from pprint import pprint
from selenium.webdriver.chrome.options import Options
from datetime import date

chrome_options = Options()
chrome_options.add_argument("--disable-popup-blocking")



#driver = webdriver.Chrome(chrome_options=chrome_options)
driver = webdriver.Firefox()
actions = ActionChains(driver)

# open the page
driver.get("https://outagemap.coned.com/external/default.html")


# get the legend Xpath
imagesInViewPortXPath = '/html/body/div[7]/div[3]/div/div/div[1]/div[3]/div/div[3]'
# get the address
addressXPath = '/html/body/div[7]/section[1]/div/div[2]/div[2]/div[2]/span[2]/span[2]'

totalOutageNum = WebDriverWait(driver, 4).until(EC.presence_of_element_located((By.XPATH, '/html/body/div[8]/div/div[1]/div[2]'))).text


# wait for 4 seconds until the icons show up --- get the intial stage
WebDriverWait(driver, 4).until(EC.presence_of_element_located((By.XPATH, '/html/body/div[7]/div[3]/div/div/div[1]/div[3]/div/div[3]/div[1]/img')))

# return to the intial stage
restoreElemment = WebDriverWait(driver, 4).until(EC.presence_of_element_located((By.XPATH, '/html/body/div[7]/ul/li[4]/button/span')))

outageImages = WebDriverWait(driver, 4).until(EC.presence_of_element_located((By.XPATH, imagesInViewPortXPath)))

ZoomOutXPath = WebDriverWait(driver, 4).until(EC.presence_of_element_located((By.XPATH, '/html/body/div[7]/ul/li[1]/button/span')))
ZoomInXPath = WebDriverWait(driver, 4).until(EC.presence_of_element_located((By.XPATH, '/html/body/div[7]/ul/li[2]/button/span')))

# get the HTML structure of icons 
#print(outageImages.get_attribute("innerHTML"))

#CrawlStatus = {}
#print(type(outageImages))
actions.move_by_offset(100, 200).perform()

address = set()
visited = {}

def zoomInUntilElements(target):
    newoutageImages = WebDriverWait(driver, 4).until(EC.presence_of_element_located((By.XPATH, imagesInViewPortXPath))).find_elements_by_tag_name('img');
    zoomCount = 0;
    while(len(newoutageImages) < target and zoomCount < 10):
        print("ZoonIn " + str(zoomCount) + " times")
        ZoomInXPath.click()
        time.sleep(3)
        zommCount = zoomCount + 1;

def writeToFile(addressToFile):
    fileName = date.today().strftime("%b-%d-%Y") + ".csv" 
    f = open(fileName, 'a+')
    with f:
        f.write(addressToFile + "###" + str(time.time()))
        f.write("\n")
        f.close()

# find the image button to click

print(totalOutageNum)


print(restoreElemment.location)

driver.maximize_window()
stepX= 500
stepY= 200
stepCountX = 30
#dirs=[[1,0], [-1,1],[-1,-1],[1,-1]]
dirs=[[1,0]]

restoreElemment.click()
# zoomOut to most
for i in range(0, 8):
    ZoomOutXPath.click()
    time.sleep(2)

#move to most left
for i in range(0, 1):
    print("move to most left") 
    ActionChains(driver).move_to_element(ZoomOutXPath).perform()
    time.sleep(1)
    ActionChains(driver).move_by_offset(500, 100).perform()
    time.sleep(1)
    #driver.get("your.site.with.dragndrop.functionality.com")
    ActionChains(driver).click_and_hold().move_by_offset(stepX * 1, 0).release().perform() 
    #actions.move_to_element_with_offset(ZoomOutXPath, stepX * dir[0], stepY* dir[1]).perform()
    time.sleep(2)
    print("move back: " ) 
    ActionChains(driver).move_by_offset((-1) * stepX * 1, 0).perform()
    time.sleep(2)
    #actions.click()
#move up 
for i in range(0, 1):
    print("move down to most down") 
    ActionChains(driver).move_to_element(ZoomOutXPath).perform()
    time.sleep(1)
    ActionChains(driver).move_by_offset(500, 100).perform()
    time.sleep(1)
    #driver.get("your.site.with.dragndrop.functionality.com")
    ActionChains(driver).click_and_hold().move_by_offset(0, (-1) * stepY).release().perform() 
    #actions.move_to_element_with_offset(ZoomOutXPath, stepX * dir[0], stepY* dir[1]).perform()
    time.sleep(2)
    print("move down back: " ) 
    ActionChains(driver).move_by_offset(0, stepY).perform()
    time.sleep(2)

moveDown = 1;
moveRight = -1
while moveDown < 100:
    start = time.time()
    for cnt in range(0, stepCountX):
        newoutageImages = WebDriverWait(driver, 10).until(EC.presence_of_element_located((By.XPATH, imagesInViewPortXPath))).find_elements_by_tag_name('img');
        print("got all icons : " + str(len(newoutageImages)))
        fail = 0
        #if time.time() - start > 10 * 60:
        #    break;
        for e in newoutageImages:
           # if(time.time() - start > 180):
           #     break;
            try:
                url = e.get_attribute('src')
                print("url: " + url)
                if 'premise/premisesclusters' in url or 'iconhighlight' in url or 'premise/premises_prob' in url:
                    e.click()
                    multipleAddress = WebDriverWait(driver, 5).until(EC.presence_of_element_located((By.XPATH, '//*[@id="info-box-content"]'))).find_elements_by_css_selector('span.premise-holder')
                    print("we have " + str(len(multipleAddress)) + " addresses")
                    i = 1;
                    repeat = 0
                    while i < len(multipleAddress):
                        newe = multipleAddress[i].find_element_by_css_selector("span.info-box-field-value")
                        toAdd = newe.get_attribute('innerHTML')
                        if toAdd not in address:
                            print("write to file: " + toAdd )
                            writeToFile(toAdd)
                            ZoomOutXPath.click()
                            ZoomInXPath.click()
                        else:
                            repeat = repeat + 1
                        i = i + 1
                    time.sleep(1)
                elif 'premise/premises_rep' in url:
                    e.click()
                    toAdd = WebDriverWait(driver, 5).until(EC.presence_of_element_located((By.XPATH, addressXPath))).get_attribute('innerHTML')
                    print("write to file single address: " + WebDriverWait(driver, 5).until(EC.presence_of_element_located((By.XPATH, addressXPath))).get_attribute('innerHTML'))
                    writeToFile(toAdd)
                    ZoomOutXPath.click()
                    ZoomInXPath.click()
                else:
                    print("new cases, cannot handle")
            except:
                print("An exception occurred")
                fail = fail + 1


        # move 
        for i in range(0, 2):
            print("move to, moveRight= " + str(moveRight)) 
            ActionChains(driver).move_to_element(ZoomOutXPath).perform()
            time.sleep(1)
            ActionChains(driver).move_by_offset(500, 100).perform()
            time.sleep(1)
            #driver.get("your.site.with.dragndrop.functionality.com")
            ActionChains(driver).click_and_hold().move_by_offset(stepX * moveRight, 0).release().perform() 
            #actions.move_to_element_with_offset(ZoomOutXPath, stepX * dir[0], stepY* dir[1]).perform()
            time.sleep(2)
            print("move back: " ) 
            ActionChains(driver).move_by_offset((-1) * stepX * moveRight, 0).perform()
            time.sleep(2)
    
    #actions.click()
    #move down
    print("move up: moveUp=" + str(moveDown)) 
    ActionChains(driver).move_to_element(ZoomOutXPath).perform()
    time.sleep(1)
    ActionChains(driver).move_by_offset(500, 100).perform()
    time.sleep(1)
    #driver.get("your.site.with.dragndrop.functionality.com")
    ActionChains(driver).click_and_hold().move_by_offset(0, stepY).release().perform() 
    #actions.move_to_element_with_offset(ZoomOutXPath, stepX * dir[0], stepY* dir[1]).perform()
    time.sleep(2)
    print("move down back: " ) 
    ActionChains(driver).move_by_offset(0, (-1) * stepY).perform()
    time.sleep(2)

    # change move directions:
    moveRight = -1 * moveRight;


print("we got: " + str(len(address)) + " addresses")

driver.close()


#recover to origina page state
#restoreElemment = WebDriverWait(driver, 4).until(EC.presence_of_element_located((By.XPATH, '/html/body/div[7]/ul/li[4]/button/span')))
#restoreElemment.click()


