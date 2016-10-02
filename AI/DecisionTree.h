#pragma once
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "Utils\Logger.h"
#include "Utils\StringUtils.h"

template<typename T>
struct DecisionNode
{
    T nodeData;
    bool isValidEnding;

    DecisionNode<T>* yesNode;
    DecisionNode<T>* noNode;
};

template<typename T>
struct SerializedNode
{
    T nodeData;
    bool isValidEnding;

    std::string thisNodeName;
    std::string yesNodeName;
    std::string noNodeName;
};

// Represents a yes-no arbitrary decision tree to randomly or algorithmically choose among various options.
template<typename T>
class DecisionTree
{
    std::function<T(std::string, bool*)> typeDeserializer;
    DecisionNode<T>* rootNode;

    bool IsLeafNode(DecisionNode<T>* node)
    {
        return node->yesNode == nullptr && node->noNode == nullptr;
    }

    bool BuildTree(std::vector<SerializedNode<T>>& givenNodes)
    {
        // The only assumptions this algorithm makes is that the first node is the root node.

        // Validate there are no duplciate names and generate the type-name mapping.
        std::map<std::string, DecisionNode<T>*> nodeNameMap;
        std::map<std::string, unsigned int> givenNodeIndex;
        std::set<std::string> namesKnown;

        for (unsigned int i = 0; i < givenNodes.size(); i++)
        {
            if (namesKnown.find(givenNodes[i].thisNodeName) == namesKnown.end())
            {
                namesKnown.insert(givenNodes[i].thisNodeName);
            }
            else
            {
                Logger::LogError("Found duplicate node name '", givenNodes[i].thisNodeName, "' in the decision tree! Cannot build the tree.");
                return false;
            }

            nodeNameMap[givenNodes[i].thisNodeName] = new DecisionNode<T>();
            nodeNameMap[givenNodes[i].thisNodeName]->nodeData = givenNodes[i].nodeData;
            nodeNameMap[givenNodes[i].thisNodeName]->isValidEnding = givenNodes[i].isValidEnding;
            nodeNameMap[givenNodes[i].thisNodeName]->yesNode = nullptr;
            nodeNameMap[givenNodes[i].thisNodeName]->noNode = nullptr;
            givenNodeIndex[givenNodes[i].thisNodeName] = i;

            if (i == 0)
            {
                // Assign the root node.
                rootNode = nodeNameMap[givenNodes[i].thisNodeName];
            }
        }

        // Now associate the pointers on all the generated nodes.
        for (auto iter = nodeNameMap.begin(); iter != nodeNameMap.end(); iter++)
        {
            // Validate all the lookups we'll do are valid.
            SerializedNode<T>* serializedNode = &givenNodes[givenNodeIndex[iter->first]];

            bool hasYesNode = (_stricmp(serializedNode->yesNodeName.c_str(), "null") != 0);
            bool hasNoNode = (_stricmp(serializedNode->noNodeName.c_str(), "null") != 0);

            if (hasYesNode && givenNodeIndex.find(serializedNode->yesNodeName) == givenNodeIndex.end())
            {
                Logger::LogError("Could not find the yes node for node ", serializedNode->thisNodeName, ", with name ", serializedNode->yesNodeName, ".");
                return false;
            }
            else if (hasNoNode && givenNodeIndex.find(serializedNode->noNodeName) == givenNodeIndex.end())
            {
                Logger::LogError("Could not find the no node for node ", serializedNode->thisNodeName, ", with name ", serializedNode->noNodeName, ".");
                return false;
            }

            // Perform the lookups.
            DecisionNode<T>* node = iter->second;
            if (hasYesNode)
            {
                node->yesNode = nodeNameMap[serializedNode->yesNodeName];
            }
            
            if (hasNoNode)
            {
                node->noNode = nodeNameMap[serializedNode->noNodeName];
            }
        }

        return true;
    }

public:
    enum Choice
    {
        THIS_ITEM,
        YES_ITEM,
        NO_ITEM
    };

    // Sets up the decision tree to deserializer a tree from a file.
    DecisionTree(std::function<T(std::string, bool*)> typeDeserializer)
        : typeDeserializer(typeDeserializer), rootNode(nullptr)
    {
    }

    bool LoadTreeFromFile(const std::string filename)
    {
        std::string file;
        if (!StringUtils::LoadStringFromFile(filename, file))
        {
            Logger::LogError("Failed to load the decision tree file '", filename, "'!");
            return false;
        }

        std::vector<std::string> lines;
        StringUtils::Split(file, StringUtils::Newline, true, lines);
        StringUtils::RemoveCommentLines(lines);

        if (lines.size() % 2 != 0)
        {
            Logger::LogError("Expected an even number of lines (excluding whitespace) with one line detailing the node structure, one detailing the node data.");
            return false;
        }

        // We parse the lines as the following:
        // <nodeId> <yesName/null> <noName/null> <isValidEnding(true/false)>
        // T nodeData
        std::vector<SerializedNode<T>> serializedNodes;
        for (unsigned int i = 0; i < lines.size() / 2; i++)
        {
            std::vector<std::string> segments;
            StringUtils::Split(lines[i * 2], StringUtils::Space, true, segments);
            if (segments.size() != 4)
            {
                Logger::LogError("Failed to find four elements in the decision tree file: ", filename, ". line: ", lines[i * 2], ".");
                return false;
            }

            bool typeDeserializeResult = false;
            SerializedNode<T> node;
            node.thisNodeName = segments[0];
            node.yesNodeName = segments[1];
            node.noNodeName = segments[2];
            node.isValidEnding = (_stricmp(segments[3].c_str(), "true") == 0);

            if (node.isValidEnding)
            {
                node.nodeData = typeDeserializer(lines[i * 2 + 1], &typeDeserializeResult);
                if (typeDeserializeResult == false)
                {
                    Logger::LogError("Could not deserialize the decision tree type-specific data: ", filename, ". line: ", lines[i * 2 + 1], ".");
                    return false;
                }
            }

            serializedNodes.push_back(node);
        }

        return BuildTree(serializedNodes);
    }

    // Runs the evaluator against the tree, returning the element.
    // The evaluator takes in the element data, whether this node is a valid ending, and true=not-null of the yes, then no node.
    T EvaluateTree(std::function<Choice(T, bool, bool, bool)> evaluator)
    {
        DecisionNode<T>* currentNode = rootNode;
        while (!IsLeafNode(currentNode))
        {
            switch (evaluator(currentNode->nodeData, currentNode->isValidEnding, currentNode->yesNode != nullptr, currentNode->noNode != nullptr))
            {
            case THIS_ITEM:
                return currentNode->nodeData;
            case YES_ITEM:
                currentNode = currentNode->yesNode;
                break;
            case NO_ITEM:
                currentNode = currentNode->noNode;
                break;
            }
        }

        // Leaf node, so we must return this data.
        return currentNode->NodeData;
    }

    // Same as EvaluateTree, except returns the whole sequence of elements walked.
    // This excludes elements without 'isValidEnding' set.
    std::vector<T> EvaluateTreeSequence(std::function<Choice(T, bool, bool, bool)> evaluator)
    {
        std::vector<T> nodeDataWalked;

        bool requestedEnding = false;
        DecisionNode<T>* currentNode = rootNode;
        while (!requestedEnding && !IsLeafNode(currentNode))
        {
            if (currentNode->isValidEnding)
            {
                nodeDataWalked.push_back(currentNode->nodeData);
            }

            switch (evaluator(currentNode->nodeData, currentNode->isValidEnding, currentNode->yesNode != nullptr, currentNode->noNode != nullptr))
            {
            case THIS_ITEM:
                requestedEnding = true;
                break;
            case YES_ITEM:
                currentNode = currentNode->yesNode;
                break;
            case NO_ITEM:
                currentNode = currentNode->noNode;
                break;
            }
        }

        // Ensure we have added the data for the leaf node.
        if (currentNode->isValidEnding && !requestedEnding)
        {
            nodeDataWalked.push_back(currentNode->nodeData);
        }

        return nodeDataWalked;
    }

    void EraseNode(DecisionNode<T>* node)
    {
        // TODO fix (very minor) memory leak. We have a leaves that can be used multiple times, so we need to keep track so we don't double delete them.
        // 
        /*if (node != nullptr)
        {
            EraseNode(node->yesNode);
            EraseNode(node->noNode);
            delete node;
        }*/
    }

    ~DecisionTree()
    {
        EraseNode(rootNode);
    }
};