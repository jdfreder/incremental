from ShowBaseGlobal import *
from DirectObject import *
from GuiGlobals import *
import GuiSign
import GuiLabel

class Sign(DirectObject):

    def __init__(self, name, label=None, font=getDefaultFont()):
        self.name = name
        # label in this case means GuiLabel
        if not label:
            self.label = GuiLabel.GuiLabel.makeSimpleTextLabel(self.name, font)
            self.label.setForegroundColor(1., 0., 0., 1.)
            self.label.setBackgroundColor(1., 1., 1., 0.)
            self.label.thaw()
        elif (type(label) == type('')):
            self.label = GuiLabel.GuiLabel.makeSimpleTextLabel(label, font)
            self.label.setForegroundColor(1., 0., 0., 1.)
            self.label.setBackgroundColor(1., 1., 1., 0.)
            self.label.thaw()
        else:
            self.label = label
        self.sign = GuiSign.GuiSign(self.name, self.label)
        self.setScale(0.1)
        self.managed = 0
	return None

    def cleanup(self):
	"""cleanup(self)
	"""
        if (self.managed):
            self.unmanage()
        self.sign = None
	return None

    def __str__(self):
        return "sign: %s contains label: %s" % (self.name, self.label.name)
    
    # accessing
    def getName(self):
        return self.name

    def setText(self, text):
        self.label.setText(text)
        
    def getLabel(self):
        return self.label
    
    def getGuiItem(self):
        return self.sign

    def getPos(self):
        self.label.getPos()
        
    def setPos(self, x, y):
        self.label.setPos(x, 0, y)
        
    def setScale(self, scale):
        self.sign.setScale(scale)

    def getWidth(self):
        return self.label.getWidth()

    def setWidth(self, width):
        self.label.setWidth(width)
        
    # actions
    def manage(self, nodepath = aspect2d):
        if nodepath:
            self.sign.manage(guiMgr, base.eventMgr.eventHandler,
                               nodepath.node())
        else:
            self.sign.manage(guiMgr, base.eventMgr.eventHandler)
        self.managed = 1

    def unmanage(self):
        self.managed = 0
        self.sign.unmanage()
