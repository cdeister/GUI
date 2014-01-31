/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "Merger.h"
#include "../Editors/MergerEditor.h"

#include "../../UI/EditorViewport.h"

#include "../Channel.h"

Merger::Merger()
    : GenericProcessor("Merger"),
      sourceNodeA(0), sourceNodeB(0), activePath(0)//, tabA(-1), tabB(-1)
{
    sendSampleCount = false;

    sendContinuousForSourceA = true;
    sendContinuousForSourceB = true;
    sendEventsForSourceA = true;
    sendEventsForSourceB = true;
}

Merger::~Merger()
{

}

AudioProcessorEditor* Merger::createEditor()
{
    editor = new MergerEditor(this, true);
    //tEditor(editor);

    //std::cout << "Creating editor." << std::endl;
    return editor;
}

void Merger::setMergerSourceNode(GenericProcessor* sn)
{

    sourceNode = sn;

    if (activePath == 0)
    {
        std::cout << "Setting source node A." << std::endl;
        sourceNodeA = sn;
    }
    else
    {
        sourceNodeB = sn;
        std::cout << "Setting source node B." << std::endl;
    }

    if (sn != nullptr)
    {
        sn->setDestNode(this);
    }
}

void Merger::switchIO(int sourceNum)
{

    //std::cout << "Switching to source number " << sourceNum << std::endl;

    activePath = sourceNum;

    if (sourceNum == 0)
    {
        sourceNode = sourceNodeA;
        //std::cout << "Source node: " << getSourceNode() << std::endl;
    }
    else
    {
        sourceNode = sourceNodeB;
        //std::cout << "Source node: " << getSourceNode() << std::endl;
    }

   // getEditorViewport()->makeEditorVisible((GenericEditor*) getEditor(), false);

}

bool Merger::stillHasSource()
{
    if (sourceNodeA == 0 || sourceNodeB == 0)
    {
        return false;
    }
    else
    {
        return true;
    }

}

bool Merger::sendEventsForSource(GenericProcessor* source)
{

    int sourceIndex = getIndexForSourceNode(source);

    if (sourceIndex == -1)
        return false;

    if (source == 0)
        return sendEventsForSourceA;
    else
        return sendEventsForSourceB;
}

bool Merger::sendContinuousForSource(GenericProcessor* source)
{

    int sourceIndex = getIndexForSourceNode(source);

    std::cout << "Source index: " <<  sourceIndex << std::endl;

    if (sourceIndex == -1)
        return false;

    if (sourceIndex == 0)
        return sendContinuousForSourceA;
    else
        return sendContinuousForSourceB;

     

}


int Merger::getIndexForSourceNode(GenericProcessor* sn)
{
    if (sn == sourceNodeA)
    {
        return 0;
    } else if (sn == sourceNodeB)
    {
        return 1;
    }

    return -1;
}

void Merger::switchIO()
{

    //std::cout << "Merger switching source." << std::endl;

    if (activePath == 0)
    {
        activePath = 1;
        sourceNode = sourceNodeB;
    }
    else
    {
        activePath = 0;
        sourceNode = sourceNodeA;
    }

}

void Merger::addSettingsFromSourceNode(GenericProcessor* sn)
{

    std::cout << "Adding settings from " << sn->getName() << std::endl;

    if (sendContinuousForSource(sn))
    {

        std::cout << "Send continous data, please!" << std::endl;

        settings.numInputs += sn->getNumOutputs();

        for (int i = 0; i < sn->channels.size(); i++)
        {
            Channel* sourceChan = sn->channels[i];
            Channel* ch = new Channel(*sourceChan);
            channels.add(ch);
        }
    } else {
        std::cout << "No continous data!" << std::endl;
    }

    if (sendEventsForSource(sn))
    {
        for (int i = 0; i < sn->eventChannels.size(); i++)
        {
            Channel* sourceChan = sn->eventChannels[i];
            Channel* ch = new Channel(*sourceChan);
            eventChannels.add(ch);
        }
    }

    settings.originalSource = sn->settings.originalSource;
    settings.sampleRate = sn->settings.sampleRate;

    settings.numOutputs = settings.numInputs;
    //settings.outputChannelNames = settings.inputChannelNames;

}

void Merger::updateSettings()
{

    // default is to get everything from sourceNodeA,
    // but this might not be ideal
    clearSettings();

    if (sourceNodeA != 0)
    {
        std::cout << "   Merger source A found." << std::endl;
        addSettingsFromSourceNode(sourceNodeA);
    }

    if (sourceNodeB != 0)
    {
        std::cout << "   Merger source B found." << std::endl;
        addSettingsFromSourceNode(sourceNodeB);
    }

    if (sourceNodeA == 0 && sourceNodeB == 0)
    {

        settings.sampleRate = getDefaultSampleRate();
        settings.numOutputs = getDefaultNumOutputs();

        for (int i = 0; i < getNumOutputs(); i++)
        {
            Channel* ch = new Channel(this, i);
            ch->sampleRate = getDefaultSampleRate();
            ch->bitVolts = getDefaultBitVolts();

            channels.add(ch);
        }

        //generateDefaultChannelNames(settings.outputChannelNames);
    }

    std::cout << "Number of merger outputs: " << getNumInputs() << std::endl;

}

void Merger::saveCustomParametersToXml(XmlElement* parentElement)
{
    XmlElement* mainNode = parentElement->createNewChildElement("MERGER");
	if (sourceNodeA!= nullptr)
		mainNode->setAttribute("NodeA",	sourceNodeA->getNodeId());
	else
		mainNode->setAttribute("NodeA",	-1);

	if (sourceNodeB != nullptr)
		mainNode->setAttribute("NodeB",	sourceNodeB->getNodeId());
	else
		mainNode->setAttribute("NodeB",	-1);
}


void Merger::loadCustomParametersFromXml()
{
	if (1)
	{
	if (parametersAsXml != nullptr)
	{
		forEachXmlChildElement(*parametersAsXml, mainNode)
		{
			if (mainNode->hasTagName("MERGER"))
			{
				int NodeAid = mainNode->getIntAttribute("NodeA");
				int NodeBid = mainNode->getIntAttribute("NodeB");

				ProcessorGraph *gr = getProcessorGraph();
				Array<GenericProcessor*> p = gr->getListOfProcessors();
				
                for (int k = 0; k < p.size(); k++)
				{
					if (p[k]->getNodeId() == NodeAid)
                    {
                        std::cout << "Setting Merger source A to " << NodeAid << std::endl;
                        switchIO(0);
						setMergerSourceNode(p[k]);
					}
                    if (p[k]->getNodeId() == NodeBid)
                    {
                        std::cout << "Setting Merger source B to " << NodeBid << std::endl;
						switchIO(1);
                        setMergerSourceNode(p[k]);
				    }
                }
				
                updateSettings();
			}
		}
	}
}
}

// void Merger::setNumOutputs(int /*outputs*/)
// {
// 	numOutputs = 0;

// 	if (sourceNodeA != 0)
// 	{
// 		std::cout << "   Merger source A found." << std::endl;
// 		numOutputs += sourceNodeA->getNumOutputs();
// 	}
// 	if (sourceNodeB != 0)
// 	{
// 		std::cout << "   Merger source B found." << std::endl;
// 		numOutputs += sourceNodeB->getNumOutputs();
// 	}

// 	std::cout << "Number of merger outputs: " << getNumOutputs() << std::endl;

// }

// void Merger::tabNumber(int t)
// {
// 	if (tabA == -1)
// 		tabA = t;
// 	else
// 		tabB = t;

// }