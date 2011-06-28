//
//  JsonToJuce.h
//  Juce Demo
//
//  Created by Aaron Leese on 5/17/11.
//  Copyright 2011 StageCraft Software. All rights reserved.
//

#include "JuceHeader.h"

#include <utility>
#include <cstdio>
#include <cassert>
#include <cstring>
#include <iostream>
#include <stdexcept>

// following includes from jsontocpp library ...
// http://sourceforge.net/projects/jsoncpp/

#include "reader.h"
#include "value.h"
#include "features.h"

/////////////////


class JsonToJuceReader : public Thread,
    public ChangeBroadcaster
{
    CriticalSection* lockToUse;
public:
    
    JsonToJuceReader() : Thread("json reader thread") 
    {
        //    
        lockToUse = 0; 
    }
    
    JsonToJuceReader(CriticalSection* lockToUse_) : Thread("json reader thread") 
    {
        lockToUse = lockToUse_;
        
    }
    
    
    void ReadJsonToValueTree(String jsonStream, juce::ValueTree & juceTreeNode)
    {
        // only one at a time ....
        if (isThreadRunning()) return; 
        
        treeToPopulate = juceTreeNode;
        
        // create the root JSON node
        Json::Reader* myJsonReader = new Json::Reader();
        rootOfJsonTree = new Json::Value();
        
        // parse the string to the jsonCPP object
        std::string stdJsonString(jsonStream.toUTF8()); 
        
        bool success = myJsonReader->parse(stdJsonString, *rootOfJsonTree);
        
        if (success) 
        {
            startThread(2);
        }
    }
    
private:
    
    ValueTree treeToPopulate;
    Json::Value* rootOfJsonTree;
    
    // Printing ...
    void PrintJSONValue( Json::Value & val ) {
        if( val.isString() ) {
            printf( "string(%s)", val.asString().c_str() ); 
        } else if( val.isBool() ) {
            printf( "bool(%d)", val.asBool() ); 
        } else if( val.isInt() ) {
            printf( "int(%d)", val.asInt() ); 
        } else if( val.isUInt() ) {
            printf( "uint(%u)", val.asUInt() ); 
        } else if( val.isDouble() ) {
            printf( "double(%f)", val.asDouble() ); 
        }
        else 
        {
            printf( "unknown type=[%d]", val.type() ); 
        }
        
        
    }
    bool PrintJSONTree( Json::Value & root, unsigned short depth /* = 0 */)  {
        depth += 1;
        printf( " {type=[%d], size=%d}", root.type(), root.size() ); 
        
        if( root.size() > 0 ) {
            printf("\n");
            for( Json::ValueIterator itr = root.begin() ; itr != root.end() ; itr++ ) {
                // Print depth. 
                for( int tab = 0 ; tab < depth; tab++) {
                    printf("-"); 
                }
                printf(" child(");
                
                Json::Value temp = itr.key();
                PrintJSONValue(temp);
                
                printf(") -");
                PrintJSONTree( *itr, depth); 
            }
            return true;
        } else {
            printf(" prop -> ");
            PrintJSONValue(root);
            printf( "\n" ); 
        }
        return true;
    }
    
    // Translating to ValueTree
    void AddChildToTree( Json::Value & val, juce::ValueTree & juceTreeNode) {
        
        if( val.isString() ) {
            
            String newChildName = String(val.asString().c_str()).retainCharacters("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");
            juceTreeNode.addChild(ValueTree(newChildName), -1 ,0);
            
        } else if( val.isBool() ) {
            juceTreeNode.addChild(ValueTree(String(val.asBool())), -1 ,0);
        } else if( val.isInt() ) {
            juceTreeNode.addChild(ValueTree(String(val.asInt())), -1 ,0);
        } else if( val.isUInt() ) { 
            juceTreeNode.addChild(ValueTree(String(val.asUInt())), -1 ,0);
        } else if( val.isDouble() ) { 
            juceTreeNode.addChild(ValueTree(String(val.asDouble())), -1 ,0);
        } else if( val.isIntegral() ) { 
            juceTreeNode.addChild(ValueTree(String(val.asDouble())), -1 ,0);
        } else if( val.isNumeric() ) { 
            juceTreeNode.addChild(ValueTree(String(val.asDouble())), -1 ,0);
        } else if( val.isArray()) { 
            juceTreeNode.addChild(ValueTree("Array"), -1 ,0);
        }
        else 
        {
            juceTreeNode.addChild(ValueTree("unknown"), -1 ,0);
        }
        
        
    }
    void AddPropertyToTree( Json::Value & val, juce::ValueTree & juceTreeNode) {
        
        if( val.isString() ) {
            
            // String newPropName = String(val.asString().c_str()).retainCharacters("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_");
            juceTreeNode.setProperty("value", val.asString().c_str(), 0); 
            
        } else if( val.isBool() ) {
            juceTreeNode.setProperty("value", val.asBool(), 0); 
        } else if( val.isInt() ) {
            juceTreeNode.setProperty("value", val.asInt(), 0);  
        } else if( val.isUInt() ) {
            juceTreeNode.setProperty("value", int(val.asUInt()), 0);  
        } else if( val.isDouble() ) {
            juceTreeNode.setProperty("value", double(val.asDouble()), 0); 
        }
        else 
        {
            printf( "unknown type=[%d]", val.type() ); 
        }
        
    }
    
    bool Read(Json::Value & root, juce::ValueTree & juceTreeNode, unsigned short depth /* = 0 */) 
    {
        depth += 1;
        
        if( root.size() > 0 ) {
            
            for( Json::ValueIterator itr = root.begin() ; itr != root.end() ; itr++ ) {
                
                Json::Value temp = itr.key();
                // this should be added as a child ... 
                AddChildToTree(temp, juceTreeNode);
                
                // dive down a level ...
                ValueTree newChild = juceTreeNode.getChild(juceTreeNode.getNumChildren()-1); // this should get us the one just added
                Read( *itr, newChild, depth); 
            }
            return true;
        } else {
            
            // this should be added as a property ... since there are no children
            AddPropertyToTree(root, juceTreeNode);
            
        }
        return true;
    }
    
    void run()
    {
        
        if (lockToUse != 0) 
            lockToUse->enter();
            
        // READ the entire tree
        Read(*rootOfJsonTree, treeToPopulate, 0); 
        
        // SAVE ....
        saveToFile();
        
        
        if (lockToUse != 0) 
            lockToUse->exit();
        
        
        
    }
    
    void saveToFile();
    
};

