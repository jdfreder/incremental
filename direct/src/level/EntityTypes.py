"""EntityTypes module: contains classes that describe Entity types"""

from SpecImports import *

class Entity:
    attribs = (
        ('type', None),
        ('name', 'unnamed'),
        ('comment', ''),
        )

class ActiveCell(Entity):
    type = 'activeCell'
    attribs = (
        ('row', 0, 'int'),
        ('col', 0, 'int'),
        ('gridId', None, 'entId', {'type':'grid'})
        )

class DirectionalCell(ActiveCell):
    type = 'directionalCell'
    attribs = (
        ('dir', [0,0], 'choice', {'choiceSet':['l','r','up','dn']}),
        )
    
class LevelMgr(Entity):
    type = 'levelMgr'
    attribs = (
        ('cogLevel', 0, 'int', {'min':0, 'max':11}),
        ('cogTrack', 'c', 'choice', {'choiceSet':['c','s','l','m']}),
        ('modelFilename', None, 'modelpath'),
        )

class EditMgr(Entity):
    type = 'editMgr'
    attribs = (
        ('requestSave', None),
        ('insertEntity', None),
        ('removeEntity', None),
        )

class LogicGate(Entity):
    type = 'logicGate'
    output = 'bool'
    attribs = (
        ('input_input1_bool', 0, 'entId', {'output':'bool'}),
        ('input_input2_bool', 0, 'entId', {'output':'bool'}),
        ('isInput1', 0, 'bool'),
        ('isInput2', 0, 'bool'),
        ('logicType', 'or', 'choice',
         {'choiceSet':['or','and','xor','nand','nor','xnor']}),
        )

class NodepathImpl:
    isNodePath = 1
    attribs = (
        ('parent', 0, 'entId', {'type':'isNodePath'}),
        ('pos', Point3(0,0,0), 'pos'),
        ('hpr', Vec3(0,0,0), 'hpr'),
        )

# Note: this covers Nodepath and DistributedNodepath
class Nodepath(Entity, NodepathImpl):
    type = 'nodepath'
    isNodePath = 1

class NodepathAttribs:
    isNodePath = 1
    attribs = (
        ('parent', 0, 'entId', {'type':'isNodePath'}),
        ('pos', Point3(0,0,0), 'pos'),
        ('hpr', Vec3(0,0,0), 'hpr'),
        )

class Zone(Entity, NodepathAttribs):
    type = 'zone'
    delAttribs = (
        'parent',
        'pos',
        'hpr',
        )
    attribs = (
        ('description', ''),
        ('modelZoneNum', None, 'int'),
        ('visibility', [], 'modelZoneList'),
        )

class CutScene(Entity):
    type = 'cutScene'
    output = 'bool'
    attribs = (
        ('pos', Point3(0,0,0), 'pos'),
        ('hpr', Vec3(0,0,0), 'hpr'),
        ('startStop', 0),
        ('effect', 'irisInOut'),
        ('motion', 'foo1'),
        ('duration', 5.0),
        )

class BarrelBase(Nodepath):
    delAttribs = (
        'hpr',
        )
    attribs = (
        ('h', 0, 'float', {'min':0, 'max':360}),
        )

class BeanBarrel(BarrelBase):
    type = 'beanBarrel'

class GagBarrel(BarrelBase):
    type = 'gagBarrel'
    attribs = (
        ('gagLevel', 0, 'int', {'min':0,'max':5}),
        ('gagTrack', 0, 'choice', {'choiceSet':range(7)}),
        )

class Switch(Entity, NodepathImpl):
    output = 'bool'
    attribs = (
        ('scale', Vec3(1), 'scale'),
        ('color', Vec4(1,1,1,1), 'color'),
        ('model', '', 'bamfilename'),
        ('input_isOn_bool', 0, 'entId', {'output':'bool'}),
        ('isOn', 0, 'bool'),
        ('secondsOn', 1, 'float'),
        )

class Button(Switch):
    type = 'button'

class Trigger(Switch):
    type = 'trigger'

class ConveyorBelt(Nodepath):
    type = 'conveyorBelt'
    attribs = (
        ('speed', 1.0, 'float'),
        ('length', 1.0, 'float'),
        ('widthScale', 1.0, 'float'),
        ('treadLength', 1.0, 'float'),
        ('treadModelPath', 'phase_7/models/cogHQ/platform1', 'bamfilename'),
        ('floorName', 'platformcollision'),
        )        
        
class Crate(Nodepath):
    type = 'crate'
    delAttribs = (
        'hpr',
        )
    attribs = (
        ('scale', Vec3(1), 'scale'),
        ('gridId', None, 'entId', {'type':'grid'}),
        ('pushable', 1, 'bool'),
        )

class Door(Entity):
    type = 'door'
    output = 'bool'
    attribs = (
        ('parent', 0, 'entId'),
        ('pos', Point3(0,0,0), 'pos'),
        ('hpr', Vec3(0,0,0), 'hpr'),
        ('scale', Vec3(1,1,1), 'scale'),
        ('color', Vec4(1,1,1,1), 'color'),
        ('model', "", 'bamfilename'),
        ('doorwayNum', 0), # Obsolete
        ('input_Lock0_bool', 0, 'entId', {'output':'bool'}),
        ('input_Lock1_bool', 0, 'entId', {'output':'bool'}),
        ('input_Lock2_bool', 0, 'entId', {'output':'bool'}),
        ('input_Lock3_bool', 0, 'entId', {'output':'bool'}),
        ('input_isOpen_bool', 0, 'entId', {'output':'bool'}),
        ('isLock0Unlocked', 0, 'bool'),
        ('isLock1Unlocked', 0, 'bool'),
        ('isLock2Unlocked', 0, 'bool'),
        ('isLock3Unlocked', 0, 'bool'),
        ('isOpen', 0, 'bool'),
        ('secondsOpen', 1, 'float'),
        )

class Grid(Nodepath):
    type = 'grid'
    delAttribs = (
        'hpr',
        )
    attribs = (
        ('cellSize', 3, 'float'),
        ('numCol', 3, 'int'),
        ('numRow', 3, 'int'),
        )

class Lift(Nodepath):
    type = 'lift'
    attribs = (
        ('duration', 1, 'float'),
        ('startPos', Point3(0,0,0), 'pos'),
        ('endPos', Point3(0,0,0), 'pos'),
        ('modelPath', '', 'bamfilename'),
        ('floorName', ''),
        ('modelScale', Vec3(1), 'scale'),
        )

class Platform(Nodepath):
    type = 'platform'
    delAttribs = (
        'pos',
        )
    attribs = (
        ('startPos', Point3(0,0,0), 'pos'),
        ('endPos', Point3(0,0,0), 'pos'),
        ('speed', 1, 'float'),
        ('waitDur', 1, 'float'),
        ('phaseShift', 0., 'float', {'min':0,'max':1}),
        )

class SinkingPlatform(Nodepath):
    type = 'sinkingPlatform'
    delAttribs = (
        'pos',
        )
    attribs = (
        ('endPos', Point3(0,0,0), 'pos'),
        ('phaseShift', 0., 'float', {'min':0,'max':1}),
        ('startPos', Point3(0,0,0), 'pos'),
        ('verticalRange', 1, 'float'),
        ('sinkRate', 1, 'float'),
        ('riseRate', 1, 'float'),
        ('speed', 1, 'float'),
        ('waitDur', 1, 'float'),
        )

class Stomper(Nodepath):
    type = 'stomper'
    attribs = (
        ('headScale', Vec3(1,1,1), 'scale'),
        ('motion', 3, 'choice',
         {'choiceSet':['linear','sinus','half sinus','slow fast']}),
        ('period', 2., 'float'),
        ('phaseShift', 0., 'float', {'min':0, 'max':1}),
        ('range', 6, 'float'),
        ('shaftScale', Vec3(1,1,1), 'scale'),
        ('soundLen', 0, 'float'),
        ('soundOn', 0, 'bool'),
        ('style', 'horizontal', 'choice',
         {'choiceSet':['horizontal', 'vertical']}),
        ('zOffset', 0, 'float'),
        )

class StomperPair(Nodepath):
    type = 'stomperPair'
    attribs = (
        ('headScale', Vec3(1,1,1), 'scale'),
        ('motion', 3, 'choice',
         {'choiceSet':['linear','sinus','half sinus','slow fast']}),
        ('period', 2., 'float'),
        ('phaseShift', 0., {'min':0, 'max':1}),
        ('range', 6, 'float'),
        ('shaftScale', Vec3(1,1,1), 'scale'),
        ('soundLen', 0, 'float'),
        ('soundOn', 0, 'bool'),
        ('stomperIds', [], 'entId', {'type':'stomper', 'num':2}),
        ('style', 'horizontal', 'choice',
         {'choiceSet':['horizontal', 'vertical']}),
        )
