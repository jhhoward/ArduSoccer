#include <math.h>
#include <vector>
#include <stdint.h>
#include <stdio.h>
#include "lodepng.h"

constexpr bool useBitPacking = false;

using namespace std;

struct SpriteFrame
{
	int width, height;
	vector<uint8_t> data;
	int xoffset;
	int offset;
};

enum EncodeMode
{
	Encode_Invalid = -1,
	Encode_Texture,
	Encode_Sprite,
	Encode_Font,
	Encode_Background
};

uint8_t texturePalette[] = 
{
	191, 191, 191,		// light grey
	255, 255, 255,		// white
	0, 0, 0,			// black
	127, 127, 127		// grey
};

uint8_t texturePaletteSprite[] = 
{
	0, 255, 255,		// cyan
	255, 255, 255,		// white
	0, 0, 0,			// black
	127, 127, 127		// grey
};

uint8_t fontPalette[] = 
{
	255, 255, 255,		// white
	0, 0, 0,			// black
	0, 0, 0,			// black
	0, 0, 0,			// black
};

EncodeMode encodeMode = Encode_Invalid;

int GetPaletteIndexFromColour(uint8_t* palette, uint8_t r, uint8_t g, uint8_t b)
{
	int bestPalette = -1;
	int bestDistance = -1;
	
	for(int n = 0; n < 4; n++)
	{
		int rdiff = (int) palette[n * 3] - (int) r;
		int gdiff = (int) palette[n * 3 + 1] - (int) g;
		int bdiff = (int) palette[n * 3 + 2] - (int) b;
		int distance = (int) sqrt((float)(rdiff * rdiff + gdiff * gdiff + bdiff * bdiff));
		
		if(bestPalette == -1 || distance < bestDistance)
		{
			bestPalette = n;
			bestDistance = distance;
		}
	}
	
	return bestPalette;
}

SpriteFrame EncodeFrame(vector<uint8_t>& data, int width, int height, int offset)
{
	int x1 = 0, y1 = 0;
	int x2 = height, y2 = height;
	bool blank;

	// Crop left side
	blank = true;
	for(int i = 0; i < height && blank; i++)
	{
		for(int j = 0; j < height && blank; j++)
		{
			int position = (offset + width * j + i) * 4;
			int index = GetPaletteIndexFromColour(texturePaletteSprite, data[position], data[position + 1], data[position + 2]);
			if(index != 0)
				blank = false;
		}
		x1 = i;
	}

	// Crop right side
	blank = true;
	for(int i = height - 1; i >= 0 && blank; i--)
	{
		for(int j = 0; j < height && blank; j++)
		{
			int position = (offset + width * j + i) * 4;
			int index = GetPaletteIndexFromColour(texturePaletteSprite, data[position], data[position + 1], data[position + 2]);
			if(index != 0)
				blank = false;
		}
		x2 = i + 1;
	}

	// Crop top side
	blank = true;
	for(int j = 0; j < height && blank; j++)
	{
		for(int i = 0; i < height && blank; i++)
		{
			int position = (offset + width * j + i) * 4;
			int index = GetPaletteIndexFromColour(texturePaletteSprite, data[position], data[position + 1], data[position + 2]);
			if(index != 0)
				blank = false;
		}
		y1 = j;
	}

	// Crop bottom side
	/*blank = true;
	for(int j = height - 1; j >= 0 && blank; j--)
	{
		for(int i = 0; i < height && blank; i++)
		{
			int position = (offset + width * j + i) * 4;
			int index = GetPaletteIndexFromColour(texturePaletteSprite, data[position], data[position + 1], data[position + 2]);
			if(index != 0)
				blank = false;
		}
		y2 = j + 1;
	}*/

	printf("%d %d -> %d %d\n", x1, y1, x2, y2);
	/*x1 = 0;
	y1 = 0;
	x2 = height;
	y2 = height;
	*/
	SpriteFrame frame;
	frame.width = x2 - x1;
	frame.height = y2 - y1;
	frame.xoffset = x1;

	for(int i = x1; i < x2; i++)
	{
		for(int j = y2 - 1; j >= y1; j--)
		{
			int position = (offset + width * j + i) * 4;
			int index = GetPaletteIndexFromColour(texturePaletteSprite, data[position], data[position + 1], data[position + 2]);
			frame.data.push_back(index);
		}
	}

	return frame;
	//int index = GetPaletteIndexFromColour(texturePaletteSprite, data[position], data[position + 1], data[position + 2]);

}

void EncodeSpriteFrames(vector<uint8_t>& data, int width, int height, vector<SpriteFrame>& frames)
{
	int offset = 0;

	for(int frame = 0; frame < width / height; frame++)
	{
		SpriteFrame newFrame = EncodeFrame(data, width, height, frame * height);
		newFrame.offset = offset;
		frames.push_back(newFrame);

		offset += newFrame.data.size();
	}

	printf("Uncompressed size: %d bytes\n", width * height / 4);
}

void OutputSpriteFile(char* filename, char* varName, vector<SpriteFrame> data)
{
	vector<uint8_t> output;

	if (useBitPacking)
	{
		uint8_t buffer = 0;
		int bufferPos = 0;

		for (int n = 0; n < data.size(); n++)
		{
			for (int i = 0; i < data[n].data.size(); i++)
			{
				int index = data[n].data[i];
				buffer |= ((index & 0x3) << bufferPos);
				bufferPos += 2;
				if (bufferPos >= 8)
				{
					output.push_back(buffer);
					buffer = 0;
					bufferPos = 0;
				}

			}
		}

		if (bufferPos > 0)
		{
			output.push_back(buffer);
		}
	}
	else
	{
		for (int n = 0; n < data.size(); n++)
		{
			for (int i = 0; i < data[n].data.size(); i++)
			{
				output.push_back(data[n].data[i]);
			}
		}
	}

	
	FILE* fs = NULL;

	char headerFilename[_MAX_PATH];
	char binFilename[_MAX_PATH];
	snprintf(headerFilename, _MAX_PATH, "%s.h", filename);
	snprintf(binFilename, _MAX_PATH, "%s.bin", filename);
	
	fopen_s(&fs, headerFilename, "w");
	
	if (fs)
	{
		fprintf(fs, "#include \"../SpriteFrame.h\"\n\n");
		fprintf(fs, "const SpriteFrame %s_frames[] PROGMEM = {\n", varName);
		for (int n = 0; n < data.size(); n++)
		{
			fprintf(fs, "\t{ %d, %d, %d, %d },\n", data[n].offset, data[n].width, data[n].height, data[n].xoffset);
		}
		fprintf(fs, "};\n\n");

		fclose(fs);
	}
	else
	{
		printf("Unable to open %s for write\n", headerFilename);
	}

	fopen_s(&fs, binFilename, "wb");

	if (fs)
	{
		fwrite(output.data(), 1, output.size(), fs);
		fclose(fs);
		//printf("Overall size: %d bytes\n", 4 * data.size() + output.size());
	}
	else
	{
		printf("Unable to open %s for write\n", binFilename);
	}
}

#define GLYPHS_X 8
#define GLYPHS_Y 8

vector<uint8_t> EncodeFont(vector<uint8_t> data, int width, int height)
{
	int glyphWidth = (width / GLYPHS_X) - 1;
	int glyphHeight = (height / GLYPHS_Y) - 1;
	vector<uint8_t> output;

	for(int y = 0; y < GLYPHS_Y; y++)
	{
		for(int x = 0; x < GLYPHS_X; x++)
		{
			uint8_t buffer = 0;
			int bufferPosition = 0;
			for(int i = 0; i < glyphWidth; i++)
			{
				for(int j = 0; j < glyphHeight; j++)
				{
					int position = ((y * (glyphHeight + 1) + j) * width + (x * (glyphWidth + 1) + i)) * 4;
					uint8_t* palette = fontPalette;
					int index = GetPaletteIndexFromColour(fontPalette, data[position], data[position + 1], data[position + 2]);

					if(index)
						buffer |= (1 << bufferPosition);
					bufferPosition++;
					if(bufferPosition == 8)
					{
						output.push_back(buffer);
						buffer = 0;
						bufferPosition = 0;
					}
				}
			}
			if(bufferPosition != 0)
			{
				output.push_back(buffer);
			}
		}
	}

	return output;
}

vector<uint8_t> EncodeImage(vector<uint8_t> data, int width, int height)
{
	vector<uint8_t> output;
	
	for(int x = 0; x < width; x++)
	{
		for(int y = 0; y < height; y++)
		{
			int position = (y * width + x) * 4;
			uint8_t* palette = encodeMode == Encode_Texture ? texturePalette : texturePaletteSprite;
			int index = GetPaletteIndexFromColour(palette, data[position], data[position + 1], data[position + 2]);
			output.push_back((uint8_t)index);
		}
	}
	
	
	return output;
}

vector<uint8_t> EncodeImagePacked(vector<uint8_t> data, int width, int height)
{
	uint8_t buffer = 0;
	int bufferPos = 0;
	vector<uint8_t> output;

	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			int position = (y * width + x) * 4;
			uint8_t* palette = encodeMode == Encode_Texture ? texturePalette : texturePaletteSprite;
			int index = GetPaletteIndexFromColour(palette, data[position], data[position + 1], data[position + 2]);
			buffer |= ((index & 0x3) << bufferPos);
			bufferPos += 2;
			if (bufferPos >= 8)
			{
				output.push_back(buffer);
				buffer = 0;
				bufferPos = 0;
			}
		}
	}

	if (bufferPos > 0)
	{
		output.push_back(buffer);
	}

	return output;
}

void OutputHeaderFile(char* filename, char* varName, vector<uint8_t> data, int width, int height, bool outputDimensions)
{
	FILE* fs = NULL;
	char headerFilename[_MAX_PATH];
	snprintf(headerFilename, _MAX_PATH, "%s.h", filename);
	
	fopen_s(&fs, headerFilename, "w");
	
	if(fs)
	{
		fprintf(fs, "const uint8_t %s[] PROGMEM = {\n\t", varName);
		for(int n = 0; n < data.size(); n++)
		{
			fprintf(fs, "0x%02x", data[n]);
			
			if(n != data.size() - 1)
			{
				fprintf(fs, ",");
				
				if(n > 0 && (n % 20) == 0)
				{
					fprintf(fs, "\n\t");
				}
			}
		}
		fprintf(fs, "\n};\n");
		fclose(fs);
		
		printf("Overall size: %d bytes\n", data.size()); 
	}
	else
	{
		printf("Unable to open %s for write\n", headerFilename);
	}
}

vector<uint8_t> EncodeBackground(vector<uint8_t> data, int width, int height)
{
	vector<uint8_t> result;

	if (width == 128 && height == 64)
	{
		for (int y = 0; y < height; y += 8)
		{
			for (int x = 0; x < width; x++)
			{
				uint8_t output = 0;

				for (int n = 0; n < 8; n++)
				{
					int index = ((y + n) * width + x) * 4;
					if (data[index] > 128)
					{
						output |= (1 << n);
					}
				}

				result.push_back(output);
			}
		}
	}

	return result;
}

void OutputBinaryFile(char* filename, char* varName, vector<uint8_t> data)
{
	FILE* fs = NULL;
	char binFilename[_MAX_PATH];
	snprintf(binFilename, _MAX_PATH, "%s.bin", filename);

	fopen_s(&fs, binFilename, "wb");

	if (fs)
	{
		fwrite(data.data(), 1, data.size(), fs);
	}
	else
	{
		printf("Unable to open %s for write\n", binFilename);
	}
}

void PrintUsage(char* processName)
{
	printf("Usage:\n"
			"%s [input.png] [output.inc.h] [varName] [texture|sprite|font]\n", processName);
}

int main2(int argc, char* argv[])
{
	if(argc != 5)
	{
		PrintUsage(argv[0]);
		return 0;
	}

	if(!strcmp(argv[4], "texture"))
	{
		encodeMode = Encode_Texture;
	}
	else if(!strcmp(argv[4], "sprite"))
	{
		encodeMode = Encode_Sprite;
	}
	else if(!strcmp(argv[4], "font"))
	{
		encodeMode = Encode_Font;
	}
	else if (!strcmp(argv[4], "background"))
	{
		encodeMode = Encode_Background;
	}

	if(encodeMode == Encode_Invalid)
	{
		PrintUsage(argv[0]);
		return 0;
	}
	
	char* filename = argv[1];
	char* outputFilename = argv[2];
	char* varName = argv[3];
	vector<uint8_t> image; 
	unsigned width, height;

	unsigned error = lodepng::decode(image, width, height, filename);
	
	if(!error)
	{
		if(encodeMode == Encode_Sprite)
		{
			vector<SpriteFrame> frames;
			EncodeSpriteFrames(image, width, height, frames);
			printf("Num frames: %d\n", frames.size());
			for(int n = 0; n < frames.size(); n++)
			{
				printf("%d : %d x %d\n", n, frames[n].width, frames[n].height);
			}
			printf("\n");
			OutputSpriteFile(outputFilename, varName, frames);
		}
		else if(encodeMode == Encode_Font)
		{
			vector<uint8_t> encoded = EncodeFont(image, width, height);
			OutputHeaderFile(outputFilename, varName, encoded, width, height, false);
		}
		else if (encodeMode == Encode_Background)
		{
			vector<uint8_t> encoded;
			encoded = EncodeBackground(image, width, height);
			OutputBinaryFile(outputFilename, varName, encoded);
		}
		else
		{
			vector<uint8_t> encoded;
			if (useBitPacking)
			{
				encoded = EncodeImagePacked(image, width, height);
			}
			else
			{
				encoded = EncodeImage(image, width, height);
			}
			OutputBinaryFile(outputFilename, varName, encoded);
		}
	}
	else
	{
		printf("Unable to open %s for read\n", filename);
	}

	return 0;
}

void EncodeBitmap(FILE* fs, const char* imagePath, const char* varName, bool outputMask = false)
{
	vector<uint8_t> image;
	unsigned width, height;
	unsigned error = lodepng::decode(image, width, height, imagePath);

	if ((height & 7) != 0)
	{
		// Needs to be multiple of 8
		printf("Error! %s needs to have a height multiple of 8!\n", imagePath);

		return;
	}

	if (!error)
	{
		fprintf(fs, "const uint8_t %s[] PROGMEM = {\n\t", varName);

		for (int y = 0; y < height; y += 8)
		{
			for (int x = 0; x < width; x++)
			{
				int value = 0;

				for (int n = 0; n < 8; n++)
				{
					int pixelIndex = ((y + n) * width + x) * 4;

					if (outputMask)
					{
						if (image[pixelIndex] != 0 || image[pixelIndex + 1] != 255 || image[pixelIndex + 2] != 255)
						{
							value |= (1 << n);
						}
					}
					else
					{
						if (image[pixelIndex] > 128)
						{
							value |= (1 << n);
						}
					}
				}

				fprintf(fs, "0x%x", value);

				if (x != width - 1 || y != height - 8)
				{
					fprintf(fs, ",");
				}
			}		
		}

		fprintf(fs, "\n};\n");

		printf("%s size: %d bytes\n", varName, width * height / 8);
	}
	else
	{
		printf("Error opening %s\n", imagePath);
	}
}

#define TILE_SIZE 8
#define BYTES_PER_TILE (TILE_SIZE * TILE_SIZE / 8)

struct Tile
{
	uint8_t pixels[BYTES_PER_TILE];
};

void EncodeBackground(FILE* fs, const char* imagePath)
{
	vector<uint8_t> image;
	unsigned width, height;
	unsigned error = lodepng::decode(image, width, height, imagePath);
	unsigned widthTiles = width / 16;
	unsigned heightTiles = height / 16;

	vector<Tile> tiles;
	vector<int> tileMap;

	if ((height & 15) != 0)
	{
		// Needs to be multiple of 16
		printf("Error! %s needs to have a height multiple of 16!\n", imagePath);
		return;
	}

	if (!error)
	{
		for (int y = 0; y < height; y += TILE_SIZE)
		{
			for (int x = 0; x < width; x += TILE_SIZE)
			{
				Tile tile;
				int pixelIndex = 0;

				for (int j = 0; j < TILE_SIZE; j += 8)
				{
					for (int i = 0; i < TILE_SIZE; i++)
					{
						uint8_t pixels = 0;

						for (int n = 0; n < 8; n++)
						{
							int imageIndex = ((y + j + n) * width + x + i) * 4;
							if (image[imageIndex] > 128)
							{
								pixels |= (1 << n);
							}
						}

						tile.pixels[pixelIndex++] = pixels;
					}
				}

				int tileIndex = -1;

				for (int n = 0; n < tiles.size(); n++)
				{
					bool match = true;

					for (int p = 0; p < BYTES_PER_TILE; p++)
					{
						if (tile.pixels[p] != tiles[n].pixels[p])
						{
							match = false;
							break;
						}
					}

					if (match)
					{
						tileIndex = n;
						break;
					}
				}

				if (tileIndex == -1)
				{
					tileIndex = tiles.size();
					tiles.push_back(tile);
				}

				tileMap.push_back(tileIndex);
			}
		}

		fprintf(fs, "const uint8_t bgTiles[] PROGMEM = {\n\t");

		for(int t = 0; t < tiles.size(); t++)
		{
			for (int p = 0; p < BYTES_PER_TILE; p++)
			{
				fprintf(fs, "0x%x", tiles[t].pixels[p]);

				if (p < 31 || t != tiles.size() - 1)
				{
					fprintf(fs, ",");
				}
			}

			if (t < tiles.size() - 1)
			{
				fprintf(fs, "\n\t");
			}
			else
			{
				fprintf(fs, "\n");
			}
		}
		fprintf(fs, "};\n\n");

		printf("Number of tiles: %d = %d bytes\n", tiles.size(), tiles.size() * BYTES_PER_TILE);

		fprintf(fs, "const uint8_t bgTileMap[] PROGMEM = {\n\t");
		for (int t = 0; t < tileMap.size(); t++)
		{
			fprintf(fs, "0x%x", tileMap[t]);
			if (t < tileMap.size() - 1)
			{
				fprintf(fs, ",");
			}
		}
		fprintf(fs, "};\n\n");

		printf("Tile map size: %d\n", tileMap.size());
	}
	else
	{
		printf("Error opening %s\n", imagePath);
	}
}

void EncodeBitmapWithMask(FILE* fs, const char* inputPath, const char* varName, const char* maskName)
{
	EncodeBitmap(fs, inputPath, varName);
	EncodeBitmap(fs, inputPath, maskName, true);
}

void EncodeBitmapFont(FILE* fs, const char* imagePath, const char* varName, int charWidth, int charHeight, int charWidthSpacing)
{
	vector<uint8_t> image;
	unsigned width, height;
	unsigned error = lodepng::decode(image, width, height, imagePath);
	vector<Tile> tiles;
	vector<int> tileMap;

	if ((height & 7) != 0)
	{
		// Needs to be multiple of 8
		printf("Error! %s needs to have a height multiple of 8!\n", imagePath);
		return;
	}

	if (!error)
	{
		int charsX = width / charWidthSpacing;
		int charsY = height / charHeight;

		fprintf(fs, "const uint8_t %s[] PROGMEM = {\n\t", varName);

		for (int cy = 0; cy < charsY; cy++)
		{
			for (int cx = 0; cx < charsX; cx++)
			{
				for (int j = 0; j < charHeight; j += 8)
				{
					for (int i = 0; i < charWidth; i++)
					{
						int values = 0;

						for (int n = 0; n < 8; n++)
						{
							int index = ((cy * charHeight + j + n) * width + cx * charWidthSpacing + i) * 4;

							if (image[index] > 128)
							{
								values |= (1 << n);
							}
						}

						fprintf(fs, "0x%x", values);

						if (i != charWidth - 1 || j != charHeight - 8 || cx != charsX - 1 || cy != charsY - 1)
						{
							fprintf(fs, ",");
						}
					}
				}
			}
		}

		fprintf(fs, "\n};\n\n");
	}
	else
	{
		printf("Error opening %s\n", imagePath);
	}
}


int main(int argc, char* argv[])
{
	FILE* fs = NULL;
	const char* outputPath = "Soccer/Generated/Pitch.inc.h";

	fopen_s(&fs, outputPath, "w");

	if (fs)
	{
		EncodeBackground(fs, "Assets/pitch.png");

		EncodeBitmap(fs, "Assets/whitePlayer.png", "playerSpriteMask", true);
		
		EncodeBitmap(fs, "Assets/whitePlayer.png", "whitePlayerSprite");
		EncodeBitmap(fs, "Assets/blackPlayer.png", "blackPlayerSprite");
		EncodeBitmap(fs, "Assets/referee.png", "refereeSprite");
		//EncodeBitmapWithMask(fs, "Assets/whitePlayer.png", "whitePlayerSprite", "whitePlayerSpriteMask");
		//EncodeBitmapWithMask(fs, "Assets/blackPlayer.png", "blackPlayerSprite", "blackPlayerSpriteMask");
		//EncodeBitmapWithMask(fs, "Assets/referee.png", "refereeSprite", "refereeSpriteMask");

		EncodeBitmapWithMask(fs, "Assets/ball.png", "ballSprite", "ballSpriteMask");

		EncodeBitmapWithMask(fs, "Assets/upperGoal.png", "upperGoalSprite", "upperGoalSpriteMask");
		EncodeBitmapWithMask(fs, "Assets/lowerGoal2.png", "lowerGoalSprite", "lowerGoalSpriteMask");

		EncodeBitmapWithMask(fs, "Assets/selectionArrow.png", "selectionArrowSprite", "selectionArrowSpriteMask");

		//EncodeBitmapFont(fs, "Assets/largeFont.png", "largeFontData", 16, 16);
		//EncodeBitmapFont(fs, "Assets/smallFont.png", "smallFontData", 8, 8);
		EncodeBitmapFont(fs, "Assets/largeFontReduced.png", "largeFontData", 15, 16, 16);
		EncodeBitmapFont(fs, "Assets/smallFontReduced.png", "smallFontData", 5, 8, 6);

		fclose(fs);
	}
	else
	{
		printf("Error opening %s for write\n", outputPath);
	}

	outputPath = "Soccer/Generated/MenuAssets.inc.h";
	fopen_s(&fs, outputPath, "w");

	if (fs)
	{
		EncodeBitmap(fs, "Assets/menuBall.png", "menuBallSprite");
		EncodeBitmap(fs, "Assets/relay.png", "relaySprite");
		EncodeBitmap(fs, "Assets/url.png", "urlSprite");
		fclose(fs);
	}
	else
	{
		printf("Error opening %s for write\n", outputPath);
	}

	return 0;
}