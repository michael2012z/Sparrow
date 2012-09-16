import os
import sys
from stat import *
from datetime import datetime, date, time

def alignedSizeOfBuffer(buffer):
	actualSize = len(buffer)
	#print "actual size is " + str(actualSize)
	quo = actualSize/(4*1024)
	mod = actualSize%(4*1024)
	if mod > 0:
		alignedSize = (quo+1)*(4*1024)
	else:
		alignedSize =  quo*(4*1024)
	#print "aligned size is " + str(alignedSize)
	return alignedSize

def printIndent(indent):
	global listStr
	for i in range(indent):
		listStr += ' '

def printNameTag(name, indent):
	global listStr
	printIndent(indent)
	listStr += "<name>\n"
	printIndent(indent+2)
	listStr += name + "\n"
	printIndent(indent)
	listStr += "</name>\n"

def printDateTag(date, indent):
	global listStr
	printIndent(indent)
	listStr += "<date>\n"
	printIndent(indent+2)
	listStr += date + "\n"
	printIndent(indent)
	listStr += "</date>\n"

def printSizeTag(size, indent):
	global listStr
	printIndent(indent)
	listStr += "<size>\n"
	printIndent(indent+2)
	listStr += size + "\n"
	printIndent(indent)
	listStr += "</size>\n"

def printOffsetTag(offset, indent):
	global listStr
	printIndent(indent)
	listStr += "<offset>\n"
	printIndent(indent+2)
	listStr += offset + "\n"
	printIndent(indent)
	listStr += "</offset>\n"

def printTopFileTag(indent):
	global listStr
	printIndent(indent)
	listStr += "<file>\n"

def printBottomFileTag(indent):
	global listStr
	printIndent(indent)
	listStr += "</file>\n"

def printTopDirTag(indent):
	global listStr
	printIndent(indent)
	listStr += "<dir>\n"

def printBottomDirTag(indent):
	global listStr
	printIndent(indent)
	listStr += "</dir>\n"


def archiveDir(dir, indent):
	if not os.path.isdir(dir):
		return

	global contentBuffer
	global contentBufferLen
	
	allList = os.listdir(dir)
	for e in allList:
		elementPath = os.path.join(dir, e)
		if os.path.isdir(elementPath):
			printTopDirTag(indent)
			printNameTag(e, indent+2)
			printDateTag(datetime.fromtimestamp(os.stat(elementPath)[ST_MTIME]).isoformat(), indent+2)
			archiveDir(elementPath, indent+2)
			printBottomDirTag(indent)
		elif os.path.isfile(elementPath):
			f = open(elementPath, "r")
			fileContent = f.read()
			actualFileSize = len(fileContent)
			alignedFileSize = alignedSizeOfBuffer(fileContent)
			contentBuffer += fileContent
			for i in range(alignedFileSize - actualFileSize):
				contentBuffer += '\0'
			f.close()
			printTopFileTag(indent)
			printNameTag(e, indent+2)
			printDateTag(datetime.fromtimestamp(os.stat(elementPath)[ST_MTIME]).isoformat(), indent+2)
			printOffsetTag(str(contentBufferLen), indent+2)
			printSizeTag(str(actualFileSize), indent+2)
			printBottomFileTag(indent)
			contentBufferLen += alignedFileSize


contentBuffer = ""
contentBufferLen = 0
listStr = ""

archiveDir("./test", 0)

alignedListStrLen = alignedSizeOfBuffer(listStr+"<!--XXXXXXXX-->\n")
listStr = "<!--" + str(alignedListStrLen) + "-->\n" + listStr

actualListStrLen = len(listStr)
if alignedListStrLen < actualListStrLen:
	print "Something is wrong."
	exit

for i in range(alignedListStrLen - actualListStrLen):
	listStr += '\0'
	
contentBuffer = listStr + contentBuffer
contentBufferLen += alignedListStrLen

archivedFile = open("ListFS.fs", "w")
archivedFile.write(contentBuffer)
archivedFile.close()

print "Actual length of listStr is " + str(actualListStrLen)
print "Aligned length of listStr is " + str(alignedListStrLen)
print "Length of archived file is " + str(contentBufferLen)

