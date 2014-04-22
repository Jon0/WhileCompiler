/*
 * FlowNode.h
 *
 *  Created on: 19/04/2014
 *      Author: asdf
 */

#ifndef FLOWNODE_H_
#define FLOWNODE_H_

#include <memory>
#include <vector>

namespace std {

class FlowNode {
public:
	FlowNode();
	virtual ~FlowNode();

private:
	vector<shared_ptr<FlowNode>> in;
	vector<shared_ptr<FlowNode>> out;

};

} /* namespace std */

#endif /* FLOWNODE_H_ */
