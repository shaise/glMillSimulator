#ifndef __stock_object_h__
#define __stock_object_h__
namespace MillSim {

    class StockObject
    {
    public:
        /// <summary>
        /// Create a stock primitive
        /// </summary>
        /// <param name="x">Stock's corner x location</param>
        /// <param name="y">Stock's corner y location</param>
        /// <param name="z">Stock's corner z location</param>
        /// <param name="l">Stock's length (along x)</param>
        /// <param name="w">Stock's width (along y)</param>
        /// <param name="h">Stock's height (along z)</param>
        StockObject(float x, float y, float z, float l, float w, float h);
        virtual ~StockObject();


        /// Calls the display list.
        virtual void render();

    private:
        unsigned int mDisplayListId;
        float mProfile[8];

    };
}

#endif