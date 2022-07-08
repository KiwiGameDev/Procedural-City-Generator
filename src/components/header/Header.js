import React from 'react'
import './header.css'
import { Box, Typography, Button, Stack } from '@mui/material'
import { width } from '@mui/system'
import { StyledEngineProvider } from '@mui/material/styles';

import {
    createTheme,
    responsiveFontSizes,
    ThemeProvider,
  } from '@mui/material/styles';
let theme = createTheme({
    
  });
theme = responsiveFontSizes(theme);


const Header = () => {
  return (
    <StyledEngineProvider injectFirst>
     <ThemeProvider theme={theme}>
      
   
        <Box className="Header_Main" alignItems='center'>

        <Box className='Header_Nested' alignItems='center' px={{xs:1, sm:2, md:3}}> 


        <Typography className='Header_SubText' variant="subtitle3" gutterBottom component="div" sx={{ alignItems:"center center", color:'white'}} margin='0 2em' align='center'>
                    Department of Software Technology <br/><br/>
        </Typography>

        <Typography className='Header_Text' variant="h2" gutterBottom component="div" sx={{ alignItems:"center center", color:'rgb(112,250,149)'}} margin='0 2em' align='center' >
                    Procedural Philippine City Generation using Real World Road Network Data
        </Typography>

        <Typography className='Header_SubText' variant="subtitle3" gutterBottom component="div" sx={{ alignItems:"center center", color:'white'}} margin='0 2em' align='center'>
                    A tool to synthetically generate a Manila-based city. Made using Unreal Engine 4.
        </Typography>

        <Stack spacing={2} direction="row" sx={{justifyContent:'center', margin:'40px 0'}}>
          <Button variant="contained" href="https://github.com/KiwiGameDev/Procedural-City-Generator" target="_blank" sx={{backgroundColor:'green'}}>View on Github</Button>
          <Button variant="contained" href="https://drive.google.com/file/d/135oUGdvAiX4vvyBEb20kgKrB93EuwAFW/view?usp=sharing" target="_blank" sx={{backgroundColor:'green'}}>Build</Button>
        </Stack>
        


        </Box>
                
    </Box>
    </ThemeProvider>   
    </StyledEngineProvider>
  )
}

export default Header