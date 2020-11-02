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
# -*- coding: utf-8 -*

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



def writeToFile(addressToFile):
    '''
    append the address###timestamp into file on current day 
    '''
    fileName = date.today().strftime("%b-%d-%Y") + ".csv" 
    f = open(fileName, 'a+')
    with f:
        f.write(addressToFile + "###" + str(time.time()))
        f.write("\n")
        f.close()

def walkToleft(step, count):
    '''
    step: pixel, length of each move
    count: how many steps we are going to move left
    '''
    for i in range(0, count):
        print("move to left") 
        ActionChains(driver).move_to_element(ZoomOutXPath).perform()
        time.sleep(1)
        ActionChains(driver).move_by_offset(500, 100).perform()
        time.sleep(1)
        ActionChains(driver).click_and_hold().move_by_offset(step * 1, 0).release().perform() 
        time.sleep(1)
        print("move back: " ) 
        ActionChains(driver).move_by_offset((-1) * step * 1, 0).perform()
        time.sleep(1)

def walkToRight(step, count):
    '''
    step: pixel, length of each move
    count: how many steps we are going to move right
    '''
    for i in range(0, count):
        print("move to right") 
        ActionChains(driver).move_to_element(ZoomOutXPath).perform()
        time.sleep(1)
        ActionChains(driver).move_by_offset(500, 100).perform()
        time.sleep(1)
        ActionChains(driver).click_and_hold().move_by_offset(step * (-1), 0).release().perform() 
        time.sleep(1)
        print("move right back: ") 
        ActionChains(driver).move_by_offset(step * 1, 0).perform()
        time.sleep(1)

def walkToDown(step, count):
    '''
    step: pixel, length of each move
    count: how many steps we are going to move down
    '''
    for i in range(0, count):
        print("move down") 
        ActionChains(driver).move_to_element(ZoomOutXPath).perform()
        time.sleep(1)
        ActionChains(driver).move_by_offset(500, 100).perform()
        time.sleep(1)
        ActionChains(driver).click_and_hold().move_by_offset(0, -1 * step).release().perform() 
        time.sleep(1)
        print("move down back: ") 
        ActionChains(driver).move_by_offset(0, step * 1).perform()
        time.sleep(1)

def walkToUp(step, count):
    '''
    step: pixel, length of each move
    count: how many steps we are going to move up
    '''
    for i in range(0, count):
        print("move up") 
        ActionChains(driver).move_to_element(ZoomOutXPath).perform()
        time.sleep(1)
        ActionChains(driver).move_by_offset(500, 100).perform()
        time.sleep(1)
        ActionChains(driver).click_and_hold().move_by_offset(0, step).release().perform() 
        time.sleep(1)
        print("move up back: ") 
        ActionChains(driver).move_by_offset(0, -1 * step).perform()
        time.sleep(1)

def restoreAndZoomToAddress(index):
    '''
    zoom indexed icon to address level 
    '''
    newoutageImages = WebDriverWait(driver, 4).until(EC.presence_of_element_located((By.XPATH, imagesInViewPortXPath))).find_elements_by_tag_name('img');
    newoutageImages[index].click()
    print("clicked the " + str(index) + " icon")
    zoomEle = WebDriverWait(driver, 30).until(EC.presence_of_element_located((By.XPATH, '//*[@id="info-box-zoom"]')))
    zoomEle.click()
    time.sleep(3)
    
    while True:
        newoutageImages = WebDriverWait(driver, 4).until(EC.presence_of_element_located((By.XPATH, imagesInViewPortXPath))).find_elements_by_tag_name('img');
        url = ""
        for ele in newoutageImages:
            try:
                ele.click()
                url = ele.get_attribute('src')
                break;
            except:
                print("icon not clickable, go to next")
        if 'premise/premises_rep' in url or 'premise/premisesclusters' in url or 'iconhighlight' in url or 'premise/premises_prob' in url:
            break;
        zoomEle = WebDriverWait(driver, 6).until(EC.presence_of_element_located((By.XPATH, '//*[@id="info-box-zoom"]')))
        try:
            zoomEle.click()
            time.sleep(3)
        except:
            print("zoom fail, will give another try")


def extractAddresses():
    '''
    extract multiple addresses or single address on current viewport 

    '''
    newoutageImages = WebDriverWait(driver, 10).until(EC.presence_of_element_located((By.XPATH, imagesInViewPortXPath))).find_elements_by_tag_name('img');
    print("got all icons : " + str(len(newoutageImages)))
    fail = 0
    start = time.time()
    for e in newoutageImages:
        if(time.time() - start > 30): # to avoid dead loop
             break;
        try:
            url = e.get_attribute('src')
            print("url: " + url)
            if 'premise/premisesclusters' in url or 'iconhighlight' in url or 'premise/premises_prob' in url:
                e.click()
                multipleAddress = WebDriverWait(driver, 5).until(EC.presence_of_element_located((By.XPATH, '//*[@id="info-box-content"]'))).find_elements_by_css_selector('span.premise-holder')
                print("we have " + str(len(multipleAddress)) + " addresses")
                i = 0;
                repeat = 0
                while i < len(multipleAddress):
                    newe = multipleAddress[i].find_element_by_css_selector("span.info-box-field-value")
                    toAdd = newe.get_attribute('innerHTML')
                    print("write to file single address: " + toAdd)
                    writeToFile(toAdd)
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


driver.maximize_window()
# here we find all icons and sort 
outageImages = WebDriverWait(driver, 4).until(EC.presence_of_element_located((By.XPATH, imagesInViewPortXPath)))
# find the image button to click 
outageImages = outageImages.find_elements_by_tag_name('img')
#sorted(outageImages, key=lambda e: e.get_attribute('src'))
#print(len(outageImages))
# for ele in outageImages:
#     print(ele.get_attribute('src')) # print the URL of each image for debug 


# quick mode, in this mode, we can guarantee extract addresses in short time. The number of addresses equal to the the number of icons in inital map
count = len(outageImages)
i = 0
while i < count:
    restoreAndZoomToAddress(i)
    extractAddresses()
    restoreElemment.click()
    time.sleep(3)
    i = i + 1
    outageImages = WebDriverWait(driver, 10).until(EC.presence_of_element_located((By.XPATH, imagesInViewPortXPath))).find_elements_by_tag_name('img')
    if i >= len(outageImages):
        break

# slow mode
#restore to initial map and we walk to the bottom-left corner to search for more
restoreElemment.click()
# zoomOut to the address level
for i in range(0, 8):
    ZoomOutXPath.click()
    time.sleep(2)


# move to most left
for i in range(0, 20):
    walkToleft(300, 2)
# move to most down
for i in range(0, 20):
    walkToDown(200, 4)



# global sweeping line to search for address
for i in range(0, 30):
    for i in range(0, 20):
        walkToRight(150, 4)
        extractAddresses()
    walkToUp(100, 4)
    extractAddresses()
    for i in range(0, 20):
        walkToleft(150, 4)
        extractAddresses()
    walkToUp(100, 4)
    extractAddresses()


driver.close()
