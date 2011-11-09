(
    function()
    {
        var algo = new Object;
        algo.apiVersion = 1;
        algo.name = "Full Rows";
        algo.author = "Heikki Junnila";

        algo.rgbMap = function(width, height, rgb, step)
        {
            var map = new Array(height);
            for (var y = 0; y < height; y++)
            {
                map[y] = new Array();
                for (var x = 0; x < width; x++)
                {
                    if (y == step)
                        map[y][x] = rgb;
                    else
                        map[y][x] = 0;
                }
            }

            return map;
        }

        algo.rgbMapStepCount = function(width, height)
        {
            return height;
        }

        return algo;
    }
)()
