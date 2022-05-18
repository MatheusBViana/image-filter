#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>

char *readline(FILE *input)
{
    char *string = calloc(1, sizeof(char));
    char c;

    c = getc(input);
    while (c != '\n' && c != '\r' && c != EOF)
    {
        string = realloc(string, (strlen(string)+1)*sizeof(char) + sizeof(char));
        strncat(string, &c, 1);
        c = getc(input);
    }
    // Finalizando string
    strncat(string, "\0", 1);

    return string;
}


struct CabecalhoDeArquivo{
    char *assinatura;        
    int TamanhoArquivo;        
    int CampoReservado;            
    int Deslocamento;       
};

struct CabecalhoDeMapaDeBits{
    int TamanhoCabecalho;             
    int LarguraDaImagem;          
    int AlturaDaImagem;         
    short NumeroDePlanos;          
    short BitPerPixel;    
    int Compressao;       
    int TamanhoDaImagem;         
    int PixelMetroHorizontal;     
    int PixelMetroVertical;      
    int NumeroCoresUsadas;          
    int NumeroCoresImportantes;      
};

struct PaletaDeCores{
    unsigned char Red;        
    unsigned char Green;        
    unsigned char Blue;        
    unsigned char CampoReservado; 
};

int checapadrao(char* string, char* pattern)
{
    regex_t parameter;
    int returnregex;

    regcomp(&parameter, pattern, REG_EXTENDED|REG_NOSUB);
    returnregex = regexec(&parameter, string, 0, NULL, 0);
    regfree(&parameter);
    
    if(returnregex != 0)
    {
        return 0;
    }
    return 1;
}


    struct CabecalhoDeArquivo LerCabecalho(FILE* file)
{
    struct CabecalhoDeArquivo CabecalhoDeArquivo;
    CabecalhoDeArquivo.assinatura = calloc(3, sizeof(char));
    fread(CabecalhoDeArquivo.assinatura, sizeof(char), 2, file);
    fread(&CabecalhoDeArquivo.TamanhoArquivo, sizeof(int), 1, file);
    fread(&CabecalhoDeArquivo.CampoReservado, sizeof(int), 1, file);
    fread(&CabecalhoDeArquivo.Deslocamento, sizeof(int), 1, file);

    return CabecalhoDeArquivo;
}

struct CabecalhoDeMapaDeBits LerMapaDeBits(FILE* file)
{
    struct CabecalhoDeMapaDeBits CabecalhoDeMapaDeBits;
    fread(&CabecalhoDeMapaDeBits.TamanhoCabecalho, sizeof(int), 1, file);
    fread(&CabecalhoDeMapaDeBits.LarguraDaImagem, sizeof(int), 1, file);
    fread(&CabecalhoDeMapaDeBits.AlturaDaImagem, sizeof(int), 1, file);
    fread(&CabecalhoDeMapaDeBits.NumeroDePlanos, sizeof(short), 1, file);
    fread(&CabecalhoDeMapaDeBits.BitPerPixel, sizeof(short), 1, file);
    fread(&CabecalhoDeMapaDeBits.Compressao, sizeof(int), 1, file);
    fread(&CabecalhoDeMapaDeBits.TamanhoDaImagem, sizeof(int), 1, file);
    fread(&CabecalhoDeMapaDeBits.PixelMetroHorizontal, sizeof(int), 1, file);
    fread(&CabecalhoDeMapaDeBits.PixelMetroVertical, sizeof(int), 1, file);
    fread(&CabecalhoDeMapaDeBits.NumeroCoresUsadas, sizeof(int), 1, file);
    fread(&CabecalhoDeMapaDeBits.NumeroCoresImportantes, sizeof(int), 1, file);
   
    return CabecalhoDeMapaDeBits;
}

// Ler a Paleta
struct PaletaDeCores* LerPaletaDeCores(FILE* Foto){
    struct PaletaDeCores* PaletaDeCores = calloc(256, sizeof(struct PaletaDeCores));
    for(int i=0; i<256; i++){
        fread(&PaletaDeCores[i].Blue, sizeof(char), 1, Foto);
        fread(&PaletaDeCores[i].Green, sizeof(char), 1, Foto);
        fread(&PaletaDeCores[i].Red, sizeof(char), 1, Foto);
        fread(&PaletaDeCores[i].CampoReservado, sizeof(char), 1, Foto);
    }

    return PaletaDeCores;
}


unsigned char** getImgMatrix(FILE* file, int largura, int altura){
unsigned char** MatrizDaImagem = calloc(altura, sizeof(char*));

if (largura % 4) 
{
largura = largura + 4 - largura % 4;
}
else
{
largura = largura;
}
    for(int i=altura-1; i>=0; i--)
    {
        MatrizDaImagem[i] = calloc(largura, sizeof(char));
        for(int j=0; j<largura; j++)
        {
            fread(&MatrizDaImagem[i][j], sizeof(char), 1, file);
        }
    }

    return MatrizDaImagem;
}

struct PaletaDeCores* Filtro(struct PaletaDeCores* PaletaOriginal, int opcao){
    struct PaletaDeCores* PaletaComFiltro = calloc(256, sizeof(struct PaletaDeCores));
    
    switch(opcao){
        case 1: // Filtro Negativo
            for(int i=0; i<256; i++)
            {
                PaletaComFiltro[i].Red = 255 - PaletaOriginal[i].Red;
                PaletaComFiltro[i].Green = 255 - PaletaOriginal[i].Green;
                PaletaComFiltro[i].Blue = 255 - PaletaOriginal[i].Blue;
            }
            break;

        case 2: // Filtro Preto e Branco
            for(int FiltroAplicado, i=0; i<256; i++)
            {
                FiltroAplicado = (int) (PaletaOriginal[i].Red + PaletaOriginal[i].Green + PaletaOriginal[i].Blue)/3;
                PaletaComFiltro[i].Red = (unsigned char)FiltroAplicado;
                PaletaComFiltro[i].Green = (unsigned char)FiltroAplicado;
                PaletaComFiltro[i].Blue = (unsigned char)FiltroAplicado;
            }
            break;
    }

    return PaletaComFiltro;
}

void ArquivoFinal(char* NomeDaImagem, struct CabecalhoDeArquivo CabecalhoDeArquivo, struct CabecalhoDeMapaDeBits CabecalhoDeMapaDeBits, struct PaletaDeCores* PaletaDeCores, unsigned char** MatrizDaImagem){
    FILE *file = fopen(NomeDaImagem, "w+");
    int largura, altura;
    altura = CabecalhoDeMapaDeBits.AlturaDaImagem;
    
        if (CabecalhoDeMapaDeBits.LarguraDaImagem % 4)
        { 
            largura = CabecalhoDeMapaDeBits.LarguraDaImagem + 4 - CabecalhoDeMapaDeBits.LarguraDaImagem % 4;
        }
        else
        {
           largura = CabecalhoDeMapaDeBits.LarguraDaImagem;
        } 


    fwrite(CabecalhoDeArquivo.assinatura, sizeof(char), 2, file);
    fwrite(&CabecalhoDeArquivo.TamanhoArquivo, sizeof(int), 1, file);
    fwrite(&CabecalhoDeArquivo.CampoReservado, sizeof(int), 1, file);
    fwrite(&CabecalhoDeArquivo.Deslocamento, sizeof(int), 1, file);
    fwrite(&CabecalhoDeMapaDeBits.TamanhoCabecalho, sizeof(int), 1, file);
    fwrite(&CabecalhoDeMapaDeBits.LarguraDaImagem, sizeof(int), 1, file);
    fwrite(&CabecalhoDeMapaDeBits.AlturaDaImagem, sizeof(int), 1, file);
    fwrite(&CabecalhoDeMapaDeBits.NumeroDePlanos, sizeof(short), 1, file);
    fwrite(&CabecalhoDeMapaDeBits.BitPerPixel, sizeof(short), 1, file);
    fwrite(&CabecalhoDeMapaDeBits.Compressao, sizeof(int), 1, file);
    fwrite(&CabecalhoDeMapaDeBits.TamanhoDaImagem, sizeof(int), 1, file);
    fwrite(&CabecalhoDeMapaDeBits.PixelMetroHorizontal, sizeof(int), 1, file);
    fwrite(&CabecalhoDeMapaDeBits.PixelMetroVertical, sizeof(int), 1, file);
    fwrite(&CabecalhoDeMapaDeBits.NumeroCoresUsadas, sizeof(int), 1, file);
    fwrite(&CabecalhoDeMapaDeBits.NumeroCoresImportantes, sizeof(int), 1, file);
   
                                                // Escrevendo a Paleta
    for(int i=0; i<256; i++)
    {
        fwrite(&PaletaDeCores[i].Blue, sizeof(char), 1, file);
        
    }
    


    for(int i=-1; i<255; i++)
    {
        fwrite(&PaletaDeCores[i].Green, sizeof(char), 1, file);

    }
     
    for (int i=-1; i<255; i++)
    {
                fwrite(&PaletaDeCores[i].Red, sizeof(char), 1, file);

    }


    for (int i=-1; i<255; i++)
    {
                fwrite(&PaletaDeCores[i].CampoReservado, sizeof(char), 1, file);

    } 
     
     
     
     
         // Escrevendo a Matriz

    for(int j = altura-1; j>=0; j--)
    {
        for(int k=0; k<largura; k++)
        {
            fwrite(&MatrizDaImagem[j][k], sizeof(char), 1, file);
        }
    }

    fclose(file);
    printf("%s\n", NomeDaImagem);
}

int main(){

    char* NomeDaImagem;
    FILE* Foto;
    int opcao;
    unsigned char** MatrizDaImagem;
    struct CabecalhoDeArquivo CabecalhoDeArquivo;
    struct CabecalhoDeMapaDeBits CabecalhoDeMapaDeBits;
    struct PaletaDeCores* PaletaOriginal;
    struct PaletaDeCores* PaletaComFiltro;
    long long int SomaDaDimensao;


    // Ler nome da imagem e opção (negativo ou preto e branco)
    printf("Insert the image filename:\n");
    NomeDaImagem = readline(stdin);
    printf("Filters:\nOption 1: Invert Colors\nOption 2: Turn picture black and white\n");
    scanf("%d", &opcao);

    // Verificar Formato do arquivo

    if(!checapadrao(NomeDaImagem, ".bmp$"))
    {
        printf("Arquivo nao eh do formato BMP");
        printf("\n");

        exit(1);
    }
    
    Foto = fopen(NomeDaImagem, "r");
    if(!Foto)
    {
        printf("Erro no arquivo");
        printf("\n");

        free(NomeDaImagem);

        exit(1);
    }       
    
    CabecalhoDeArquivo = LerCabecalho(Foto);

    // Verificar formato do arquivo
    if(strcmp(CabecalhoDeArquivo.assinatura, "BM"))
    {
        printf("Arquivo nao eh do formato BMP");
        printf("\n");

        exit(1);
    }
    CabecalhoDeMapaDeBits = LerMapaDeBits(Foto);
    PaletaOriginal = LerPaletaDeCores(Foto);
    MatrizDaImagem = getImgMatrix(Foto, CabecalhoDeMapaDeBits.LarguraDaImagem, CabecalhoDeMapaDeBits.AlturaDaImagem);
    
    PaletaComFiltro = Filtro(PaletaOriginal, opcao);

    
    // Modificando o nome do Arquivo
    switch(opcao)
    {
        case 1:
            NomeDaImagem = realloc(NomeDaImagem, strlen(NomeDaImagem) + strlen("Negativo") + sizeof(char));
            NomeDaImagem = strtok(NomeDaImagem, ".");
            NomeDaImagem = strcat(NomeDaImagem, "Negativo.bmp");
            break;

        case 2:
            NomeDaImagem = realloc(NomeDaImagem, strlen(NomeDaImagem) + strlen("PretoBranco") + sizeof(char));
            NomeDaImagem = strtok(NomeDaImagem, ".");
            NomeDaImagem = strcat(NomeDaImagem, "PretoBranco.bmp");
            break;
    }
    
    // Saídas do programa - Cabeçalho do Arquivo
    printf("CABECALHO:\n");
    printf("Iniciais: %s\n", CabecalhoDeArquivo.assinatura);
    printf("Tamanho do arquivo: %d\n", CabecalhoDeArquivo.TamanhoArquivo);    
    printf("Reservado: %d\n", CabecalhoDeArquivo.CampoReservado);    
    printf("Deslocamento, em bytes, para o inicio da area de dados: %d\n", CabecalhoDeArquivo.Deslocamento);

    // Saídas do programa - Cabeçalho do Mapa de Bits
    printf("Tamanho em bytes do segundo cabecalho: %d\n", CabecalhoDeMapaDeBits.TamanhoCabecalho);
    printf("Resolucao: %d x %d\n", CabecalhoDeMapaDeBits.LarguraDaImagem, CabecalhoDeMapaDeBits.AlturaDaImagem);
    printf("Numero de planos: %d\n", CabecalhoDeMapaDeBits.NumeroDePlanos);
    printf("Bits por pixel: %d\n", CabecalhoDeMapaDeBits.BitPerPixel);
    printf("Compressao usada: %d\n", CabecalhoDeMapaDeBits.Compressao);
    printf("Tamanho imagem: %d\n", CabecalhoDeMapaDeBits.TamanhoDaImagem);
    printf("Resolucao horizontal: %d pixel por metro\n", CabecalhoDeMapaDeBits.PixelMetroHorizontal);
    printf("Resolucao Vertical: %d pixel por metro\n", CabecalhoDeMapaDeBits.PixelMetroVertical);
    printf("Numero de cores usadas: %d\n", CabecalhoDeMapaDeBits.NumeroCoresUsadas);
    printf("Numero de cores importantes: %d\n", CabecalhoDeMapaDeBits.NumeroCoresImportantes);

    // Printar a Paleta original
    printf("PALETA ORIGINAL:\n");
    for(int i=0; i<256; i++)
    {
        printf("Paleta[%d]: R:%d G:%d B:%d\n", i, PaletaOriginal[i].Red, PaletaOriginal[i].Green, PaletaOriginal[i].Blue);
    } 
    
    // Printar a Paleta Nova
    printf("PALETA NOVA:\n");

    for(int i=0; i<256; i++)
    {
        printf("Paleta[%d]: R:%d G:%d B:%d\n", i, PaletaComFiltro[i].Red, PaletaComFiltro[i].Green, PaletaComFiltro[i].Blue);
    } 
    
    for(int i=0; i<CabecalhoDeMapaDeBits.AlturaDaImagem; i++)
    {
        SomaDaDimensao = 0;
        for(int j=0; j<CabecalhoDeMapaDeBits.LarguraDaImagem; j++)
        {
            SomaDaDimensao += MatrizDaImagem[i][j];
        }
        if (CabecalhoDeMapaDeBits.LarguraDaImagem % 4)
        {
            SomaDaDimensao = SomaDaDimensao - (4 - CabecalhoDeMapaDeBits.LarguraDaImagem % 4);
        }
        else
        {
            SomaDaDimensao = SomaDaDimensao;
        }
        
        printf("Soma linha %d: %lld\n", i, SomaDaDimensao);
    }  

    ArquivoFinal(NomeDaImagem, CabecalhoDeArquivo, CabecalhoDeMapaDeBits, PaletaComFiltro, MatrizDaImagem);

    fclose(Foto); 

    for(int i=0; i<CabecalhoDeMapaDeBits.AlturaDaImagem; i++)
    {
        free(MatrizDaImagem[i]);
    }  

    free(PaletaOriginal);
    free(PaletaComFiltro);
    free(MatrizDaImagem);
    free(NomeDaImagem);
    free(CabecalhoDeArquivo.assinatura);
    return 0;
}