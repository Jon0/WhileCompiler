/*
 * Type.cpp
 *
 *  Created on: 16/03/2014
 *      Author: remnanjona
 */

#include <map>

#include "Type.h"
#include "Var.h"

namespace std {


bool ListType::contains( const Type &other ) const {
	if ( elem_type && other.isList() ) {
		ListType &other_list = (ListType &)other;

		// consider nulls(empty list) a subtype
		return other_list.elem_type == NULL || elem_type->contains(*other_list.elem_type);
	}
	if ( elem_type && other.isUnion() ) {

		// [int|real] contains [int]|[real]
		UnionType &other_u = (UnionType &)other;
		return other_u.containedByList(*elem_type);

	}
	return *this == other;
}

vector<shared_ptr<Type>> UnionType::normalise( const vector<shared_ptr<Type>> types ) {
	cout << "normalise union" << endl;
	vector<shared_ptr<Type>> sets;

	// copy element of the union
	for (auto &a: types){
		if (a->isRecord()) {
			RecordType &thisrec = (RecordType &)*a;

			// find a record in sets with matching names
			bool found = false;
			for (auto other : sets) {
				if (thisrec.sameNames(*other)) {
					// merge records
					map<string, Var> result;
					if ( other->isRecord() ) {
						RecordType &otherrec = (RecordType &)*other;

						for (auto entry : thisrec.getElems()) {
							map<string, Var> other_elems = otherrec.getElems();
							auto it = other_elems.find( entry.first );
							if ( it != other_elems.end() ) {
								vector<shared_ptr<Type>> un;
								un.push_back(entry.second.type());
								un.push_back((*it).second.type());
								result.insert( {(*it).first, Var(shared_ptr<Type>( new UnionType(un) ), (*it).second)} );
							}
							else {
								throw runtime_error("normalise failed");
							}
						}

						otherrec.modify(result);
					}
					else {
						throw runtime_error("normalise failed");
					}


					found = true;
					break;
				}
			}

			if (!found) {
				sets.push_back(a->data()); // make copy
			}
		}
		else {
			sets.push_back(a->data()); // make copy
		}
	}

	return sets;
}

} /* namespace std */
