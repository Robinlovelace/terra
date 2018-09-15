// Based on  public-domain code by Darel Rex Finley, 2007
// http://alienryderflex.com/polygon_fill/

using namespace std;
#include <vector>
#include "spat.h"
  
std::vector<double> rasterize_polygon(std::vector<double> r, double value, std::vector<double> pX, std::vector<double> pY, unsigned nrows, unsigned ncols, double xmin, double ymax, double rx, double ry) {

	unsigned n = pX.size();
	std::vector<unsigned> nCol(n);
  
	for (size_t row=0; row<nrows; row++) {
  
		double y = ymax - (row+0.5) * ry;
		
		//  Build a list of nodes.
		unsigned nodes = 0; 
		size_t j = n-1;
		for (size_t i=0; i<n; i++) {
			if (((pY[i] < y) && (pY[j] >= y)) || ((pY[j] < y) && (pY[i] >= y))) {
			//	nCol[nodes++]=(int)  (((pX[i] - xmin + (y-pY[i])/(pY[j]-pY[i]) * (pX[j]-pX[i])) + 0.5 * rx ) / rx); 
				double nds = (((pX[i] - xmin + (y-pY[i])/(pY[j]-pY[i]) * (pX[j]-pX[i])) + 0.5 * rx ) / rx); 
				nds = nds < 0 ? 0 : nds;
				nCol[nodes] = (unsigned) nds;
				nodes++;
			}
			j = i; 
		}
		
		std::sort(nCol.begin(), nCol.begin()+nodes);
		
		//  Fill the pixels between node pairs.
		for (size_t i=0; i < nodes; i+=2) {
			if (nCol[i] >= ncols) break;
			if (nCol[i+1] > 0) {
				if (nCol[i] < 0) nCol[i] = 0 ;
				if (nCol[i+1] > ncols) nCol[i+1] = ncols;
				int ncell = ncols * row;
				//for (size_t col = nCol[i]; col < nCol[i+1]; col++) {
				for (size_t col = nCol[i]; col < nCol[i+1]; col++) {
					r[col + ncell] = value;
				}
			}
		}
	}
	return(r);
}



SpatRaster SpatRaster::rasterizePolygons(SpatPolygons p, double background, string filename, bool overwrite) {

	SpatRaster out = *this;
	out.values.resize(0);
  	out.writeStart(filename, overwrite);
	
	double value = 1;
	double resx = xres();
	double resy = yres();
	
	SpatPoly poly;
	SpatPolyPart part;
	
	unsigned n = p.size();
	
	for (size_t i = 0; i < out.bs.n; i++) {
		std::vector<double> v(out.bs.nrows[i] * ncol, background);
		
		for (size_t j = 0; j < n; j++) {
			
			poly = p.getPoly(j);
			
			//value = p.getAtt(j);
			//if (std::isnan(value)) { value = j;}
			value = j+1;
			
			unsigned np = poly.size();
			
			for (size_t k = 0; k < np; k++) {
				part = poly.getPart(k);
				if (part.hasHoles()) {
					std::vector<double> vv = rasterize_polygon(v, value, part.x, part.y, nrow, ncol, extent.xmin, extent.ymax, resx, resy);
					for (size_t h=0; h < part.nHoles(); h++) {
						vv = rasterize_polygon(vv, background, part.xHole[h], part.yHole[h], nrow, ncol, extent.xmin, extent.ymax, resx, resy);
					}
					for (size_t q=0; q < vv.size(); q++) {
						if (vv[q] != background) {
							v[q] = vv[q];
						}
					}
				} else {
					v = rasterize_polygon(v, value, part.x, part.y, nrow, ncol, extent.xmin, extent.ymax, resx, resy);
				}
			}
		}
		out.writeValues(v, out.bs.row[i]);
	}
	out.writeStop();
	
	return(out);

}
	